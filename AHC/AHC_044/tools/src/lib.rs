#![allow(non_snake_case, unused_macros)]

use itertools::Itertools;
use proconio::input;
use rand::prelude::*;
use std::{io::prelude::*, io::BufReader, process::ChildStdout};
use svg::node::{
    element::{Group, Line, Path, Polygon, Rectangle, Style, Title},
    Text,
};

// ==================================================================================================================
// ===== 1. デフォルトのライブラリ部分
// ==================================================================================================================
pub trait SetMinMax {
    fn setmin(&mut self, v: Self) -> bool;
    fn setmax(&mut self, v: Self) -> bool;
}
impl<T> SetMinMax for T
where
    T: PartialOrd,
{
    fn setmin(&mut self, v: T) -> bool {
        *self > v && {
            *self = v;
            true
        }
    }
    fn setmax(&mut self, v: T) -> bool {
        *self < v && {
            *self = v;
            true
        }
    }
}

#[macro_export]
macro_rules! mat {
	($($e:expr),*) => { Vec::from(vec![$($e),*]) };
	($($e:expr,)*) => { Vec::from(vec![$($e),*]) };
	($e:expr; $d:expr) => { Vec::from(vec![$e; $d]) };
	($e:expr; $d:expr $(; $ds:expr)+) => { Vec::from(vec![mat![$e $(; $ds)*]; $d]) };
}

// ==================================================================================================================
// ===== 2. 入力関連
// ==================================================================================================================
#[derive(Clone, Debug)]
pub struct Input {
    pub n: usize,
    pub l: usize,
    pub t: Vec<usize>,
}

// 入力形式
impl std::fmt::Display for Input {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        writeln!(f, "{} {}", self.n, self.l)?;
        writeln!(f, "{}", self.t.iter().join(" "))?;
        Ok(())
    }
}

// 入力のパース
pub fn parse_input(f: &str) -> Input {
    let f = proconio::source::once::OnceSource::from(f);
    input! {
        from f,
        n: usize,
        l: usize,
        t: [usize; n]
    }
    Input { n, l, t }
}

// 入力のチェック (lb 以上 ub 以下か？)
pub fn read<T: Copy + PartialOrd + std::fmt::Display + std::str::FromStr>(
    token: Option<&str>,
    lb: T,
    ub: T,
) -> Result<T, String> {
    if let Some(v) = token {
        if let Ok(v) = v.parse::<T>() {
            if v < lb || ub < v {
                Err(format!("Out of range: {}", v))
            } else {
                Ok(v)
            }
        } else {
            Err(format!("Parse error: {}", v))
        }
    } else {
        Err("Unexpected EOF".to_owned())
    }
}

// ==================================================================================================================
// ===== 3. 出力関連
// ==================================================================================================================
pub struct Output {
    pub out: Vec<(usize, usize)>,
}

// 出力のパース
pub fn parse_output(input: &Input, f: &str) -> Result<Output, String> {
    let mut out = vec![];
    let mut tokens = f.split_whitespace().peekable();
    while tokens.peek().is_some() {
        let a = read(tokens.next(), 0, input.n - 1)?;
        let b = read(tokens.next(), 0, input.n - 1)?;
        out.push((a, b));
    }
    if out.len() != input.n {
        return Err("Number of lines in output is not N".to_owned());
    }
    Ok(Output { out })
}

// ==================================================================================================================
// ===== 4. 入力生成関連
// ==================================================================================================================
pub fn gen(seed: u64) -> Input {
    let mut rng = rand_chacha::ChaCha20Rng::seed_from_u64(seed);
    let n = 100;
    let l = 500000;
    let mut t = vec![0; n];

    // Second Loop
    loop {
        let mut sum = 0;
        for i in 0..n - 1 {
            t[i] = rng.gen_range(0..=10000 as i32) as usize;
            sum += t[i];
        }
        if l - 10000 <= sum && sum <= l {
            t[n - 1] = l - sum;
            break;
        }
    }

    // Return
    Input { n, l, t }
}

// ==================================================================================================================
// ===== 5. スコア計算
// ==================================================================================================================
pub fn compute_score(input: &Input, out: &Output) -> (i64, String) {
    let (mut score, err, _) = compute_score_details(input, &out.out);
    if err.len() > 0 {
        score = 0;
    }
    (score, err)
}

// 実際のスコアの計算
pub fn compute_score_details(input: &Input, out: &[(usize, usize)]) -> (i64, String, ()) {
    let mut counts = vec![0; input.n];
    let mut pos = 0;

    // シミュレーション開始
    for _ in 0..input.l {
        let a = out[pos].0;
        let b = out[pos].1;
        counts[pos] += 1;
        if counts[pos] % 2 == 1 {
            pos = a;
        } else {
            pos = b;
        }
    }

    // スコアの計算
    let mut score = (2 * input.l) as i64;
    for i in 0..input.n {
        let diff = counts[i] - input.t[i] as i64;
        score -= diff.abs();
    }
    return (score, "".to_owned(), ());
}

// ==================================================================================================================
// ===== 6. ビジュアライザ
// ==================================================================================================================
pub fn color(mut val: f64) -> String {
    val.setmin(1.0);
    val.setmax(0.0);
    let (r, g, b) = if val < 0.5 {
        let x = val * 2.0;
        (
            0. * (1.0 - x) + 255. * x,
            112. * (1.0 - x) + 255. * x,
            192. * (1.0 - x) + 255. * x,
        )
    } else {
        let x = val * 2.0 - 1.0;
        (
            255. * (1.0 - x) + 208. * x,
            255. * (1.0 - x) + 64. * x,
            255. * (1.0 - x) + 64. * x,
        )
    };
    format!(
        "#{:02x}{:02x}{:02x}",
        r.round() as i32,
        g.round() as i32,
        b.round() as i32
    )
}

pub fn rect(x: usize, y: usize, w: usize, h: usize, fill: &str) -> Rectangle {
    Rectangle::new()
        .set("x", x)
        .set("y", y)
        .set("width", w)
        .set("height", h)
        .set("fill", fill)
}

pub fn group(title: String) -> Group {
    Group::new().add(Title::new().add(Text::new(title)))
}

pub fn vis_default(input: &Input, out: &Output) -> (i64, String, String) {
    let (mut score, err, svg) = vis(input, &out.out, input.l);
    if err.len() > 0 {
        score = 0;
    }
    (score, err, svg)
}

pub fn vis(input: &Input, out: &[(usize, usize)], t: usize) -> (i64, String, String) {
    // initialization
    let W = 800;
    let H = 600;
    let (score, err, _) = compute_score_details(input, &out);
    let mut doc = svg::Document::new()
        .set("id", "vis")
        .set("viewBox", (-5, -5, W + 10, H + 10))
        .set("width", W + 10)
        .set("height", H + 10)
        .set("style", "background-color:white");
    doc = doc.add(Style::new(format!(
        "text {{text-anchor: middle;dominant-baseline: central;}}"
    )));

    // calculate counts
    let mut counts = vec![0; input.n];
    let mut pos = 0;
    for _ in 0..t {
        let a = out[pos].0;
        let b = out[pos].1;
        counts[pos] += 1;
        if counts[pos] % 2 == 1 {
            pos = a;
        } else {
            pos = b;
        }
    }

    // visualization: sections
    let mut groups = vec![];
    for i in 0..100 {
        groups.push(
            group(format!(
                "A[{}] = {}, B[{}] = {}, T[{}] = {}, t[{}] = {}",
                i, out[i].0, i, out[i].1, i, input.t[i], i, counts[i]
            ))
            .set("class", "vis_item"),
        )
    }
    let mut grouptexts = vec![];
    let MAGIC = 1.0 / (2.0 + 8.0 / 2.0_f64.log10());
    for i in 0..5 {
        for j in 0..20 {
            let id = i * 20 + j;
            let diff = counts[id] as i64 - input.t[id] as i64;
            let mut rate = 0.5;
            if diff != 0 {
                rate = 0.5 + MAGIC + (0.5 - MAGIC) / 4.0 * (diff.abs() as f64).log10();
            }
            if diff < 0 {
                rate = 1.0 - rate;
            }
            groups[id] = groups[id].clone().add(
                rect(
                    i * W / 5 + W / 15,
                    20 + j * (H - 20) / 20 + 1,
                    W / 5 - W / 15,
                    (H - 20) / 20 - 2,
                    &color(rate),
                )
                .set("stroke", "black")
                .set("stroke-width", 1),
            );
            let x = i * W / 5 + W / 15;
            let y = 20 + j * (H - 20) / 20 + (H - 20) / 40;
            if counts[id] % 2 == 0 {
                grouptexts.push(
                    svg::node::element::Text::new()
                        .add(Text::new(format!("{:02}", out[id].0)))
                        .set("x", x + W / 60)
                        .set("y", y)
                        .set("fill", "black")
                        .set("font-size", (H - 20) / 40),
                );
                grouptexts.push(
                    svg::node::element::Text::new()
                        .add(Text::new(format!("{:02}", out[id].1)))
                        .set("x", x + W / 28)
                        .set("y", y + (H - 20) / 160)
                        .set("fill", "black")
                        .set("font-size", (H - 20) / 80),
                );
            } else {
                grouptexts.push(
                    svg::node::element::Text::new()
                        .add(Text::new(format!("{:02}", out[id].0)))
                        .set("x", x + W / 80)
                        .set("y", y + (H - 20) / 160)
                        .set("fill", "black")
                        .set("font-size", (H - 20) / 80),
                );
                grouptexts.push(
                    svg::node::element::Text::new()
                        .add(Text::new(format!("{:02}", out[id].1)))
                        .set("x", x + W / 32)
                        .set("y", y)
                        .set("fill", "black")
                        .set("font-size", (H - 20) / 40),
                );
            }
            let count_color = if counts[id] == input.t[id] {
                "#996600"
            } else {
                "black"
            };
            let font_weight = if counts[id] == input.t[id] {
                "bold"
            } else {
                "none"
            };
            grouptexts.push(
                svg::node::element::Text::new()
                    .add(Text::new(format!("{:04}", counts[id])))
                    .set("x", x + W / 13)
                    .set("y", y)
                    .set("fill", count_color)
                    .set("font-weight", font_weight)
                    .set("font-size", (H - 20) / 50),
            );
            grouptexts.push(
                svg::node::element::Text::new()
                    .add(Text::new(format!("/ {:04}", input.t[id])))
                    .set("x", x + W / 200 * 23)
                    .set("y", y + H / 160)
                    .set("fill", count_color)
                    .set("font-weight", font_weight)
                    .set("font-size", (H - 20) / 80),
            );
        }
    }
    let subx = (pos / 20) * W / 5;
    let suby = 20 + (pos % 20) * (H - 20) / 20 + (H - 20) / 40;
    doc = doc.add(
        Polygon::new()
            .set(
                "points",
                format!(
                    "{} {}, {} {}, {} {}",
                    subx + W / 100,
                    suby - (H - 20) / 60,
                    subx + W / 100,
                    suby + (H - 20) / 60,
                    subx + W / 40,
                    suby
                ),
            )
            .set("fill", "red"),
    );
    for g in groups {
        doc = doc.add(g);
    }
    for g in grouptexts {
        doc = doc.add(g);
    }
    for i in 0..5 {
        for j in 0..20 {
            let id = i * 20 + j;
            let x = i * W / 5 + W / 15;
            let y = 20 + j * (H - 20) / 20 + (H - 20) / 40;
            doc = doc.add(
                Path::new()
                    .set(
                        "d",
                        format!(
                            "M {} {} A 7 7 0 0 0 {} {} M {} {}",
                            x - 9,
                            y - 5,
                            x - 23,
                            y - 5,
                            x - 9,
                            y - 5
                        ),
                    )
                    .set("fill", "black"),
            );
            doc = doc.add(rect(x - 25, y - 4, 18, 18, "color"));
            doc = doc.add(rect(x - 27, y - 4, 22, 5, "color"));
            doc = doc.add(rect(x - 27, y - 4, 22, 5, "color"));
            doc = doc.add(
                svg::node::element::Text::new()
                    .add(Text::new(format!("{:02}", id)))
                    .set("x", x - 16)
                    .set("y", y + 5)
                    .set("fill", "white")
                    .set("font-size", (H - 20) / 45),
            );
        }
    }

    // visualization: background
    for i in 0..5 {
        let x = i * W / 5 + W / 15;
        doc = doc.add(
            svg::node::element::Text::new()
                .add(Text::new("NEXT"))
                .set("x", x + 19)
                .set("y", 10)
                .set("fill", "black")
                .set("font-size", 10),
        );
        doc = doc.add(
            svg::node::element::Text::new()
                .add(Text::new("COUNT / T[i]"))
                .set("x", x + 75)
                .set("y", 10)
                .set("fill", "black")
                .set("font-size", 10),
        );
        doc = doc.add(
            Line::new()
                .set("x1", x + 38)
                .set("y1", 0)
                .set("x2", x + 38)
                .set("y2", H)
                .set("stroke", "black")
                .set("stroke-width", 0.3)
                .set("stroke-dasharray", "2,2"),
        );
    }

    (score, err, doc.to_string())
}

// ==================================================================================================================
// ===== 7. 一行読む関数: インタラクション用のライブラリ
// ==================================================================================================================
fn read_line(stdout: &mut BufReader<ChildStdout>, local: bool) -> Result<String, String> {
    loop {
        let mut out = String::new();
        match stdout.read_line(&mut out) {
            Ok(0) | Err(_) => {
                return Err(format!("Your program has terminated unexpectedly"));
            }
            _ => (),
        }
        if local {
            print!("{}", out);
        }
        let v = out.trim();
        if v.len() == 0 || v.starts_with("#") {
            continue;
        }
        return Ok(v.to_owned());
    }
}

// ==================================================================================================================
// ===== 8. 最後の実行部分
// ==================================================================================================================
pub fn exec(p: &mut std::process::Child, local: bool) -> Result<i64, String> {
    let mut input = String::new();
    std::io::stdin().read_to_string(&mut input).unwrap();
    let input = parse_input(&input);
    let mut stdin = std::io::BufWriter::new(p.stdin.take().unwrap());
    let mut stdout = std::io::BufReader::new(p.stdout.take().unwrap());

    // インタラクション: 入力を書き込む
    let _ = writeln!(stdin, "{} {}", input.n, input.l);
    let _ = stdin.flush();
    let _ = writeln!(stdin, "{}", input.t.iter().join(" "));
    let _ = stdin.flush();

    // インタラクション: 出力を読み込む
    let mut out = vec![];
    for _ in 0..input.n {
        let line = read_line(&mut stdout, local)?;
        let mut tokens = line.split_whitespace();
        let a = read(tokens.next(), 0, input.n - 1)?;
        let b = read(tokens.next(), 0, input.n - 1)?;
        out.push((a, b));
        if tokens.next().is_some() {
            return Err(format!("Too many output: {}", line));
        }
    }

    // スコアの計算
    p.wait().unwrap();
    let final_output = Output { out: out };
    Ok(compute_score(&input, &final_output).0)
}
