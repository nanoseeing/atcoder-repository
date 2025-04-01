use rand::distributions::WeightedIndex;
use rand::prelude::*;
use std::fs::File;
use std::io::{self, BufRead, Write};
use std::time::Instant;

const DEBUG: bool = false;
const MAX_TIME: f64 = 1.92;
// const MAX_TIME: f64 = 100.0;
const INIT_TEMP: f64 = 1000.0;
const MIN_TEMP: f64 = 30.0;

const MIN_L: i32 = 2_000;
const MAX_L: i32 = 5_0000;

const IS_CHANGE: bool = true;
const MIN_CHANGE: i32 = 3;
const MAX_CHANGE: i32 = 20;

fn weighted_choice(weights: &[i32]) -> Option<usize> {
    let mut rng = thread_rng();
    let dist = WeightedIndex::new(weights).ok()?;
    Some(dist.sample(&mut rng))
}

fn cooling_schedule(initial_temp: f64, min_temp: f64, elapsed_time: f64, max_time: f64) -> f64 {
    let lambda_param = (initial_temp / min_temp).ln() / max_time;
    min_temp + (initial_temp - min_temp) * (-lambda_param * elapsed_time).exp()
}

fn exponential_schedule(init: f64, obj: f64, elapsed_time: f64, max_time: f64) -> f64 {
    let lambda_param = (obj / init).ln() / max_time;
    init * (lambda_param * elapsed_time).exp()
}

fn calc_score(x: &(Vec<i32>, Vec<i32>), n: usize, l: usize, t: &Vec<i32>) -> f64 {
    let mut counts = vec![0; n];
    let mut pre_n = 0;
    for _ in 0..l {
        counts[pre_n] += 1;
        pre_n = if counts[pre_n] % 2 != 0 {
            x.0[pre_n] as usize
        } else {
            x.1[pre_n] as usize
        };
    }
    let mut ret = 1_000_000
        - counts
            .iter()
            .zip(t.iter())
            .map(|(&c, &t)| (t - c).abs())
            .sum::<i32>();
    return ret as f64;
}

fn calc_score_simulate(x: &(Vec<i32>, Vec<i32>), test_l: usize, l: usize, t: &Vec<i32>) -> f64 {
    let mut counts = vec![0; x.0.len()];
    let mut pre_n = 0;
    for _ in 0..test_l {
        counts[pre_n] += 1;
        pre_n = if counts[pre_n] % 2 != 0 {
            x.0[pre_n] as usize
        } else {
            x.1[pre_n] as usize
        };
    }
    let rate = test_l as f64 / l as f64;
    let rate_inv = l as f64 / test_l as f64;
    1_000_000.0
        - counts
            .iter()
            .zip(t.iter())
            .map(|(&c, &t)| ((t as f64 * rate - c as f64).abs() * rate_inv))
            .sum::<f64>()
}

fn simulated_annealing(
    initial_x: (Vec<i32>, Vec<i32>),
    initial_temp: f64,
    min_temp: f64,
    max_time: f64,
    n: usize,
    l: usize,
    t: &Vec<i32>,
    display: bool,
) -> ((Vec<i32>, Vec<i32>), f64) {
    let mut x = initial_x;
    let mut best_x = x.clone();
    let mut best_cost = -calc_score(&x, n, l, t);
    let mut current_cost = best_cost;
    let start_time = Instant::now();
    let mut rng = thread_rng();

    let mut iter = 0;
    while start_time.elapsed().as_secs_f64() < max_time {
        iter += 1;
        let elapsed_time = start_time.elapsed().as_secs_f64();
        let temp = cooling_schedule(initial_temp, min_temp, elapsed_time, max_time);
        let now_l =
            exponential_schedule(MIN_L as f64, MAX_L as f64, elapsed_time, max_time) as usize;
        let change_n =
            exponential_schedule(MIN_CHANGE as f64, MAX_CHANGE as f64, elapsed_time, max_time)
                as usize;
        // change n回だけ変更を行う
        let new_x = neighbor_function(&x, n, l, t);
        if IS_CHANGE {
            for _ in 1..change_n {
                let new_x = neighbor_function(&new_x, n, l, t);
            }
        }
        let new_cost = -calc_score_simulate(&new_x, now_l.min(l), l, t);
        let delta_cost = new_cost - current_cost;

        if delta_cost < 0.0 || rng.gen::<f64>() < (-delta_cost / temp).exp() {
            x = new_x;
            current_cost = new_cost;
        }

        if current_cost < best_cost {
            best_x = x.clone();
            best_cost = current_cost;
        }

        if display && iter % 1000 == 0 {
            println!(
                "iter: {}, cost: {}, best_cost: {}, temp: {}, now_l: {}",
                iter, current_cost, best_cost, temp, now_l
            );
        }
    }
    (best_x, best_cost)
}

fn linear_schedule(init: f64, obj: f64, elapsed_time: f64, max_time: f64) -> f64 {
    init + (obj - init) * (elapsed_time / max_time)
}

fn neighbor_function(
    x: &(Vec<i32>, Vec<i32>),
    n: usize,
    l: usize,
    t: &Vec<i32>,
) -> (Vec<i32>, Vec<i32>) {
    let mut rng = thread_rng();
    let (mut an, mut bn) = (x.0.clone(), x.1.clone());
    if rng.gen::<f64>() < 0.5 {
        let i = rng.gen_range(0..n);
        let j = rng.gen_range(0..n);
        if rng.gen::<f64>() < 0.5 {
            an.swap(i, j);
        } else {
            bn.swap(i, j);
        }
    } else {
        let sample_n = rng.gen_range(0..n);
        // let sample_n = weighted_choice(&t).unwrap();
        let i = rng.gen_range(0..n);
        if rng.gen::<f64>() < 0.5 {
            an[i] = sample_n as i32;
        } else {
            bn[i] = sample_n as i32;
        }
    }
    (an, bn)
}

fn get_random_ans(n: usize, l: usize, t: &Vec<i32>) -> (Vec<i32>, Vec<i32>) {
    let mean_t = l as f64 / n as f64;
    let mut random_list = Vec::new();
    let mut rng = thread_rng();

    for (i, &ti) in t.iter().enumerate() {
        let r = (ti as f64 / mean_t * 2.0).round().max(1.0) as usize;
        for _ in 0..r {
            random_list.push(i as i32);
        }
    }

    while random_list.len() < n * 2 {
        random_list.push(rng.gen_range(0..n) as i32);
    }
    random_list.truncate(n * 2);
    random_list.shuffle(&mut rng);

    let an: Vec<i32> = random_list
        .iter()
        .enumerate()
        .filter_map(|(i, &v)| if i % 2 == 0 { Some(v) } else { None })
        .collect();
    let bn: Vec<i32> = random_list
        .iter()
        .enumerate()
        .filter_map(|(i, &v)| if i % 2 != 0 { Some(v) } else { None })
        .collect();

    (an, bn)
}

fn file_input(file_path: &str) -> io::Result<(usize, usize, Vec<i32>)> {
    let file = File::open(file_path)?;
    let mut lines = io::BufReader::new(file).lines();
    let first_line = lines
        .next()
        .unwrap()?
        .split_whitespace()
        .map(|s| s.parse::<usize>().unwrap())
        .collect::<Vec<_>>();
    let n = first_line[0];
    let l = first_line[1];
    let t = lines
        .next()
        .unwrap()?
        .split_whitespace()
        .map(|s| s.parse::<i32>().unwrap())
        .collect();
    Ok((n, l, t))
}

fn inline_input() -> io::Result<(usize, usize, Vec<i32>)> {
    let mut input = String::new();
    io::stdin().read_line(&mut input)?;
    let values: Vec<usize> = input
        .trim()
        .split_whitespace()
        .map(|s| s.parse().unwrap())
        .collect();
    let (n, l) = (values[0], values[1]);

    let mut input = String::new();
    io::stdin().read_line(&mut input)?;
    let t: Vec<i32> = input
        .trim()
        .split_whitespace()
        .map(|s| s.parse().unwrap())
        .collect();

    Ok((n, l, t))
}

fn answer_output_file(an: &Vec<i32>, bn: &Vec<i32>, file_path: &str) -> io::Result<()> {
    let mut file = File::create(file_path)?;
    for (&a, &b) in an.iter().zip(bn.iter()) {
        writeln!(file, "{} {}", a, b)?;
    }
    Ok(())
}

fn answer_output_inline(an: &Vec<i32>, bn: &Vec<i32>) {
    for (&a, &b) in an.iter().zip(bn.iter()) {
        println!("{} {}", a, b);
    }
}

fn soleve(file_n: i32) {
    let input_file = format!("./tools/in/{:04}.txt", file_n);
    let output_file = format!("./output/{:04}.txt", file_n);
    let (n, l, t) = if DEBUG {
        writeln!(io::stderr(), "file: {}", input_file).unwrap();
        file_input(&input_file).unwrap()
    } else {
        inline_input().unwrap()
    };

    let initial_x = get_random_ans(n, l, &t);
    let (best_x, best_cost) =
        simulated_annealing(initial_x, INIT_TEMP, MIN_TEMP, MAX_TIME, n, l, &t, DEBUG);

    if DEBUG {
        let _ = answer_output_file(&best_x.0, &best_x.1, &output_file);
        println!("Best cost: {}", calc_score(&best_x, n, l, &t));
    } else {
        answer_output_inline(&best_x.0, &best_x.1);
    }
}

fn main() {
    if DEBUG {
        let file_n = 100;
        for i in 0..file_n {
            soleve(i);
        }
    } else {
        soleve(0);
    }
}
