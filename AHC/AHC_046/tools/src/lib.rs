#![allow(non_snake_case, unused_macros)]

use proconio::input;
use rand::prelude::*;
use std::ops::RangeBounds;
use svg::node::element::{Group, Line, Rectangle, Style, Symbol, Title, Use};

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

#[derive(Clone, Debug)]
pub struct Input {
    N: usize,
    ps: Vec<(usize, usize)>,
}

impl std::fmt::Display for Input {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        writeln!(f, "{} {}", self.N, self.ps.len())?;
        for (x, y) in &self.ps {
            writeln!(f, "{} {}", x, y)?;
        }
        Ok(())
    }
}

pub fn parse_input(f: &str) -> Input {
    let f = proconio::source::once::OnceSource::from(f);
    input! {
        from f,
        N: usize, M: usize,
        ps: [(usize, usize); M],
    }
    Input { N, ps }
}

pub fn read<T: Copy + PartialOrd + std::fmt::Display + std::str::FromStr, R: RangeBounds<T>>(
    token: Option<&str>,
    range: R,
) -> Result<T, String> {
    if let Some(v) = token {
        if let Ok(v) = v.parse::<T>() {
            if !range.contains(&v) {
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

pub const DIJ: [(usize, usize); 4] = [(!0, 0), (1, 0), (0, !0), (0, 1)];
pub const DIR: [char; 4] = ['U', 'D', 'L', 'R'];

#[derive(Clone, Debug, Copy)]
pub enum Action {
    Move(usize),
    Slide(usize),
    Alter(usize),
}

pub struct Output {
    pub out: Vec<Action>,
}

pub fn parse_output(input: &Input, f: &str) -> Result<Output, String> {
    let mut ss = f.split_whitespace().peekable();
    let mut out = vec![];
    while ss.peek().is_some() {
        let a = read(ss.next(), 'A'..='Z')?;
        let d = read(ss.next(), 'A'..='Z')?;
        let Some(d) = DIR.iter().position(|&dir| dir == d) else {
            return Err(format!("Invalid direction: {}", d));
        };
        let a = match a {
            'M' => Action::Move(d),
            'S' => Action::Slide(d),
            'A' => Action::Alter(d),
            _ => return Err(format!("Invalid action: {}", a)),
        };
        out.push(a);
        if out.len() > 2 * input.N * input.ps.len() {
            return Err(format!("Too many actions: {}", out.len()));
        }
    }
    Ok(Output { out })
}

pub fn gen(seed: u64) -> Input {
    let mut rng = rand_chacha::ChaCha20Rng::seed_from_u64(seed);
    let N = 20;
    let M = N * 2;
    let mut ps = vec![];
    for i in 0..N {
        for j in 0..N {
            ps.push((i, j));
        }
    }
    ps.shuffle(&mut rng);
    ps.truncate(M);
    Input { N, ps }
}

pub fn compute_score(input: &Input, out: &Output) -> (i64, String) {
    let (mut score, err, _) = compute_score_details(input, &out.out);
    if err.len() > 0 {
        score = 0;
    }
    (score, err)
}

#[derive(Clone, Debug)]
pub struct State {
    pub pi: usize,
    pub pj: usize,
    pub done: usize,
    pub block: Vec<Vec<bool>>,
    pub prev: (usize, usize),
}

pub fn compute_score_details(input: &Input, out: &[Action]) -> (i64, String, State) {
    let mut state = State {
        pi: input.ps[0].0,
        pj: input.ps[0].1,
        done: 1,
        block: mat![false; input.N; input.N],
        prev: input.ps[0],
    };
    for action in out {
        state.prev = (state.pi, state.pj);
        match *action {
            Action::Move(dir) => {
                let (di, dj) = DIJ[dir];
                let pi = state.pi + di;
                let pj = state.pj + dj;
                if pi >= input.N || pj >= input.N {
                    return (0, format!("Out of range (M {})", DIR[dir]), state);
                }
                if state.block[pi][pj] {
                    return (0, format!("Blocked (M {})", DIR[dir]), state);
                }
                state.pi = pi;
                state.pj = pj;
            }
            Action::Slide(dir) => {
                let (di, dj) = DIJ[dir];
                loop {
                    let pi = state.pi + di;
                    let pj = state.pj + dj;
                    if pi >= input.N || pj >= input.N || state.block[pi][pj] {
                        break;
                    }
                    state.pi = pi;
                    state.pj = pj;
                }
            }
            Action::Alter(dir) => {
                let (di, dj) = DIJ[dir];
                let i = state.pi + di;
                let j = state.pj + dj;
                if i >= input.N || j >= input.N {
                    return (0, format!("Out of range (A {})", DIR[dir]), state);
                }
                state.block[i][j] ^= true;
            }
        }
        if state.done < input.ps.len() && input.ps[state.done] == (state.pi, state.pj) {
            state.done += 1;
        }
    }
    let score = if state.done < input.ps.len() {
        state.done as i64
    } else {
        input.ps.len() as i64 + (2 * input.N * input.ps.len()) as i64 - out.len() as i64
    };
    (score, String::new(), state)
}

/// 0 <= val <= 1
pub fn color(mut val: f64) -> String {
    val.setmin(1.0);
    val.setmax(0.0);
    let (r, g, b) = if val < 0.5 {
        let x = val * 2.0;
        (
            30. * (1.0 - x) + 144. * x,
            144. * (1.0 - x) + 255. * x,
            255. * (1.0 - x) + 30. * x,
        )
    } else {
        let x = val * 2.0 - 1.0;
        (
            144. * (1.0 - x) + 255. * x,
            255. * (1.0 - x) + 30. * x,
            30. * (1.0 - x) + 70. * x,
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
    Group::new().add(Title::new(title))
}

pub fn vis_default(input: &Input, out: &Output) -> (i64, String, String) {
    let (mut score, err, svg) = vis(input, &out.out);
    if err.len() > 0 {
        score = 0;
    }
    (score, err, svg)
}

// https://www.svgrepo.com/svg/17487/skater-silhouette
const SKATER: &str = r#"<g>
    <path d="M411.474,47.191c0-26-21.286-47.191-47.401-47.191c-25.98,0-47.124,21.2-47.124,47.191
        c0,26.163,21.144,47.258,47.124,47.258C390.188,94.449,411.474,73.363,411.474,47.191z"/>
    <path d="M477.302,209.313h-82.533l-37.973-85.881c-0.459-1.434-1.291-3.271-1.77-4.112c-0.144-0.268-0.239-0.411-0.239-0.536
        l-0.669-1.568l-0.144,0.134c-7.057-14.927-21.783-25.599-39.244-26.249c-3.414-0.201-6.493,0.134-9.572,0.784L97.805,140.588
        c-12.546,2.955-20.416,15.319-17.404,27.951c2.821,12.575,15.128,20.292,27.693,17.461l138.436-32.904L141.343,326.607H64.757
        h-0.125H41.73c1.3-3.826,0.995-8.166-1.32-11.848c-4.131-6.607-12.708-8.568-19.182-4.457c-6.598,4.064-8.549,12.643-4.504,19.25
        c3.146,4.963,8.893,7.248,14.372,6.244l2.984,11.742c-1.128,0.354-2.238,0.803-3.309,1.453
        c-6.417,4.055-8.444,12.633-4.313,19.107c3.137,5.105,8.97,7.371,14.449,6.281l2.964,11.639c-1.147,0.354-2.276,0.793-3.337,1.463
        c-6.675,4.055-8.559,12.613-4.514,19.24c3.156,5.039,9.151,7.305,14.679,6.176l2.974,11.705c-1.195,0.334-2.362,0.775-3.461,1.434
        c-6.598,4.189-8.568,12.758-4.523,19.24c4.073,6.607,12.642,8.568,19.326,4.514c5.24-3.309,7.478-9.678,5.891-15.463
        c0.382-0.191,0.803-0.199,1.176-0.439c3.089-1.77,5.04-4.906,5.642-8.119l14.927-45.43l0.526-1.818c0,0,68.84-0.258,69.356-0.258
        c13.828,0,25.676-3.807,32.675-15.377l31.872-52.812l71.03,70.686l-27.014,106.326c-2.544-0.729-5.308-0.775-8.004,0.037
        c-7.411,2.42-11.523,10.289-9.18,17.615c2.496,7.449,10.347,11.379,17.605,9.16c6.33-2.084,10.26-8.234,9.734-14.602h11.905
        c-0.172,1.826,0.01,3.719,0.612,5.574c2.361,7.449,10.279,11.523,17.547,9.17c6.378-2.123,10.261-8.328,9.62-14.744h12.058
        c-0.21,1.893-0.057,3.873,0.574,5.832c2.353,7.336,10.203,11.447,17.614,9.037c6.388-1.961,10.337-8.301,9.505-14.879h11.57
        c0.211,0,0.383-0.115,0.593-0.125c-0.268,2.008-0.162,4.104,0.526,6.148c2.419,7.268,10.279,11.342,17.538,8.98
        c7.334-2.363,11.389-10.098,9.026-17.414c-2.123-6.742-8.778-10.596-15.453-9.609c0-0.01,0.01-0.02,0.01-0.029
        c0-5.24-3.28-8.912-7.334-11.121l-44.963-24.883l21.602-85.211c0,0,5.374-18.006-11.666-34.615l-58.111-58.197
        c0,0,31.087-51.055,50.987-83.845l23.179,52.489c1.817,4.198,4.925,7.602,8.568,9.954c0,0.125,0.277,0.125,0.277,0.335
        c3.662,2.333,8.052,3.777,12.766,3.777h97.04c12.958,0.144,23.371-10.461,23.495-23.237
        C500.463,219.717,490.078,209.505,477.302,209.313z"/>
</g>"#;

pub fn vis(input: &Input, out: &[Action]) -> (i64, String, String) {
    let D = 600 / input.N;
    let W = input.N * D;
    let (score, err, state) = compute_score_details(input, &out);
    let mut doc = svg::Document::new()
        .set("id", "vis")
        .set("viewBox", (-5, -5, W + 10, W + 10))
        .set("width", W + 10)
        .set("height", W + 10)
        .set("style", "background-color:white");
    doc = doc.add(Style::new(format!(
        "text {{text-anchor: middle;dominant-baseline: central;}}"
    )));
    doc = doc.add(
        Symbol::new()
            .set("id", "skater")
            .set("viewBox", (0, 0, 514.962, 514.962))
            .add(svg::node::Blob::new(SKATER)),
    );
    let mut ids = mat![!0; input.N; input.N];
    for k in 0..input.ps.len() {
        let (i, j) = input.ps[k];
        ids[i][j] = k;
    }
    for i in 0..input.N {
        for j in 0..input.N {
            let rect = rect(
                j * D,
                i * D,
                D,
                D,
                if state.block[i][j] { "gray" } else { "white" },
            )
            .set("stroke", "black")
            .set("stroke-width", 1);
            let mut group = group(format!("({}, {})", i, j)).add(rect);
            if ids[i][j] != !0 && ids[i][j] >= state.done {
                group = group.add(
                    svg::node::element::Text::new(format!("{}", ids[i][j]))
                        .set("x", j * D + D / 2)
                        .set("y", i * D + D / 2)
                        .set("font-size", D / 2)
                        .set(
                            "fill",
                            if ids[i][j] == state.done {
                                "red"
                            } else {
                                "black"
                            },
                        ),
                );
            }
            if (i, j) == (state.pi, state.pj) {
                let skater = Use::new()
                    .set("x", j * D + D / 8)
                    .set("y", i * D + D / 8)
                    .set("width", D * 3 / 4)
                    .set("height", D * 3 / 4)
                    .set("href", "#skater")
                    .set("fill", "#ff8080");
                group = group.add(skater);
            }
            doc = doc.add(group);
        }
    }
    if (state.pi, state.pj) != state.prev {
        doc = doc.add(
            Line::new()
                .set("x1", (state.prev.1) * D + D / 2)
                .set("y1", (state.prev.0) * D + D / 2)
                .set("x2", (state.pj) * D + D / 2)
                .set("y2", (state.pi) * D + D / 2)
                .set("stroke", "#8080ff")
                .set("stroke-width", 2),
        );
    }
    (score, err, doc.to_string())
}
