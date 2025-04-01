#![allow(non_snake_case)]

use std::fs::File;
use std::io::prelude::*;

use tools::*;

fn main() {
    if std::env::args().len() != 4 {
        eprintln!(
            "Usage: {} <input> <output> <result>",
            std::env::args().nth(0).unwrap()
        );
        return;
    }
    let in_file = std::env::args().nth(1).unwrap();
    let out_file = std::env::args().nth(2).unwrap();
    let input = std::fs::read_to_string(&in_file).unwrap_or_else(|_| {
        eprintln!("no such file: {}", in_file);
        std::process::exit(1)
    });
    let output = std::fs::read_to_string(&out_file).unwrap_or_else(|_| {
        eprintln!("no such file: {}", out_file);
        std::process::exit(1)
    });
    let input = parse_input(&input);
    let out = parse_output(&input, &output);
    let (score, err, svg) = match out {
        Ok(out) => vis_default(&input, &out),
        Err(err) => (0, err, String::new()),
    };
    if err.len() > 0 {
        println!("{}", err);
        println!("Score = {}", 0);
    } else {
        println!("Score = {}", score);
    }
    // let vis = format!("<html><body>{}</body></html>", svg);
    // std::fs::write("vis.html", &vis).unwrap();

    let mut write_score = score;
    if score == 0 {
        write_score = 1_000_000_000;
    }
    let result_file = std::env::args().nth(3).unwrap();
    let file = File::create(result_file);
    let content = format!("{}", write_score);
    let _ = file.expect("REASON").write_all(content.as_bytes());
}
