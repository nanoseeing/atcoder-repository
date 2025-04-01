#![allow(non_snake_case, unused_macros)]

use itertools::Itertools;
use proconio::input;
use rand::prelude::*;
use std::io::Read;
use std::io::Write;
use std::io::{prelude::*, BufReader};
use std::ops::RangeBounds;
use std::process::ChildStdout;
use svg::node::{
    element::{Circle, Group, Line, Rectangle, Style, Title},
    Text,
};

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

// https://github.com/rust-lang-ja/ac-library-rs/blob/master/src/dsu.rs

/// A Disjoint set union (DSU) with union by size and path compression.
///
/// See: [Zvi Galil and Giuseppe F. Italiano, Data structures and algorithms for disjoint set union problems](https://core.ac.uk/download/pdf/161439519.pdf)
///
/// In the following documentation, let $n$ be the size of the DSU.
///
/// # Example
///
/// ```
/// use ac_library::Dsu;
/// use proconio::{input, source::once::OnceSource};
///
/// input! {
///     from OnceSource::from(
///         "5\n\
///          3\n\
///          0 1\n\
///          2 3\n\
///          3 4\n",
///     ),
///     n: usize,
///     abs: [(usize, usize)],
/// }
///
/// let mut dsu = Dsu::new(n);
/// for (a, b) in abs {
///     dsu.merge(a, b);
/// }
///
/// assert!(dsu.same(0, 1));
/// assert!(!dsu.same(1, 2));
/// assert!(dsu.same(2, 4));
///
/// assert_eq!(
///     dsu.groups()
///         .into_iter()
///         .map(|mut group| {
///             group.sort_unstable();
///             group
///         })
///         .collect::<Vec<_>>(),
///     [&[0, 1][..], &[2, 3, 4][..]],
/// );
/// ```
pub struct Dsu {
    n: usize,
    // root node: -1 * component size
    // otherwise: parent
    parent_or_size: Vec<i32>,
}

impl Dsu {
    /// Creates a new `Dsu`.
    ///
    /// # Constraints
    ///
    /// - $0 \leq n \leq 10^8$
    ///
    /// # Complexity
    ///
    /// - $O(n)$
    pub fn new(size: usize) -> Self {
        Self {
            n: size,
            parent_or_size: vec![-1; size],
        }
    }

    // `\textsc` does not work in KaTeX
    /// Performs the Uɴɪᴏɴ operation.
    ///
    /// # Constraints
    ///
    /// - $0 \leq a < n$
    /// - $0 \leq b < n$
    ///
    /// # Panics
    ///
    /// Panics if the above constraints are not satisfied.
    ///
    /// # Complexity
    ///
    /// - $O(\alpha(n))$ amortized
    pub fn merge(&mut self, a: usize, b: usize) -> usize {
        assert!(a < self.n);
        assert!(b < self.n);
        let (mut x, mut y) = (self.leader(a), self.leader(b));
        if x == y {
            return x;
        }
        if -self.parent_or_size[x] < -self.parent_or_size[y] {
            std::mem::swap(&mut x, &mut y);
        }
        self.parent_or_size[x] += self.parent_or_size[y];
        self.parent_or_size[y] = x as i32;
        x
    }

    /// Returns whether the vertices $a$ and $b$ are in the same connected component.
    ///
    /// # Constraints
    ///
    /// - $0 \leq a < n$
    /// - $0 \leq b < n$
    ///
    /// # Panics
    ///
    /// Panics if the above constraint is not satisfied.
    ///
    /// # Complexity
    ///
    /// - $O(\alpha(n))$ amortized
    pub fn same(&mut self, a: usize, b: usize) -> bool {
        assert!(a < self.n);
        assert!(b < self.n);
        self.leader(a) == self.leader(b)
    }

    /// Performs the Fɪɴᴅ operation.
    ///
    /// # Constraints
    ///
    /// - $0 \leq a < n$
    ///
    /// # Panics
    ///
    /// Panics if the above constraint is not satisfied.
    ///
    /// # Complexity
    ///
    /// - $O(\alpha(n))$ amortized
    pub fn leader(&mut self, a: usize) -> usize {
        assert!(a < self.n);
        if self.parent_or_size[a] < 0 {
            return a;
        }
        self.parent_or_size[a] = self.leader(self.parent_or_size[a] as usize) as i32;
        self.parent_or_size[a] as usize
    }

    /// Returns the size of the connected component that contains the vertex $a$.
    ///
    /// # Constraints
    ///
    /// - $0 \leq a < n$
    ///
    /// # Panics
    ///
    /// Panics if the above constraint is not satisfied.
    ///
    /// # Complexity
    ///
    /// - $O(\alpha(n))$ amortized
    pub fn size(&mut self, a: usize) -> usize {
        assert!(a < self.n);
        let x = self.leader(a);
        -self.parent_or_size[x] as usize
    }

    /// Divides the graph into connected components.
    ///
    /// The result may not be ordered.
    ///
    /// # Complexity
    ///
    /// - $O(n)$
    pub fn groups(&mut self) -> Vec<Vec<usize>> {
        let mut leader_buf = vec![0; self.n];
        let mut group_size = vec![0; self.n];
        for i in 0..self.n {
            leader_buf[i] = self.leader(i);
            group_size[leader_buf[i]] += 1;
        }
        let mut result = vec![Vec::new(); self.n];
        for i in 0..self.n {
            result[i].reserve(group_size[i]);
        }
        for i in 0..self.n {
            result[leader_buf[i]].push(i);
        }
        result
            .into_iter()
            .filter(|x| !x.is_empty())
            .collect::<Vec<Vec<usize>>>()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn dsu_works() {
        let mut d = Dsu::new(4);
        d.merge(0, 1);
        assert!(d.same(0, 1));
        d.merge(1, 2);
        assert!(d.same(0, 2));
        assert_eq!(d.size(0), 3);
        assert!(!d.same(0, 3));
        assert_eq!(d.groups(), vec![vec![0, 1, 2], vec![3]]);
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
    pub N: usize,
    pub M: usize,
    pub Q: usize,
    pub L: usize,
    pub W: usize,
    pub G: Vec<usize>,
    pub range: Vec<(usize, usize, usize, usize)>,
    pub xy: Vec<(usize, usize)>,
}

impl std::fmt::Display for Input {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        writeln!(f, "{} {} {} {} {}", self.N, self.M, self.Q, self.L, self.W)?;
        for i in 0..self.M {
            write!(f, "{}", self.G[i])?;
            if i < self.M - 1 {
                write!(f, " ")?;
            } else {
                writeln!(f)?;
            }
        }
        for i in 0..self.N {
            writeln!(
                f,
                "{} {} {} {}",
                self.range[i].0, self.range[i].1, self.range[i].2, self.range[i].3
            )?;
        }
        for i in 0..self.N {
            writeln!(f, "{} {}", self.xy[i].0, self.xy[i].1)?;
        }
        Ok(())
    }
}

pub fn parse_input(f: &str) -> Input {
    let f = proconio::source::once::OnceSource::from(f);
    input! {
        from f,
        N: usize, M: usize, Q: usize, L: usize, W: usize,
        G: [usize; M],
        range: [(usize, usize, usize, usize); N],
        xy: [(usize, usize); N],
    }
    Input {
        N,
        M,
        Q,
        L,
        W,
        G,
        range,
        xy,
    }
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

pub struct Output {
    pub queries: Vec<String>,
    pub outputs: Vec<String>,
}

pub fn parse_output(_input: &Input, f: &str) -> Result<Output, String> {
    let mut queries = vec![];
    let mut outputs = vec![];
    let mut after_output = false;
    for line in f.lines() {
        let line = line.trim();
        if line.len() == 0 {
            continue;
        } else if after_output {
            outputs.push(line.to_owned());
        } else if line.starts_with('?') {
            // query
            queries.push(line.to_owned());
        } else if line.starts_with('!') {
            // output
            if line != "!" {
                return Err(format!("Illegal output format: {}", line));
            }
            after_output = true;
        } else {
            return Err(format!("Unknown line: {}", line));
        }
    }

    Ok(Output { queries, outputs })
}

pub fn gen(seed: u64, fixM: Option<usize>, fixL: Option<usize>, fixW: Option<usize>) -> Input {
    let mut rng = rand_chacha::ChaCha20Rng::seed_from_u64(seed);
    let N = 800 as usize;
    let M_rng = rng.gen_range(1.0f32..=20.0) as f64;
    let mut M = (M_rng.powf(2.0)).floor() as usize;
    if let Some(fixM) = fixM {
        M = fixM;
    }
    let Q = 400 as usize;
    let mut L = rng.gen_range(3i32..=15) as usize;
    if let Some(fixL) = fixL {
        L = fixL;
    }
    let mut W = rng.gen_range(500i32..=2500) as usize;
    if let Some(fixW) = fixW {
        W = fixW;
    }

    let mut G = vec![];
    let mut A = vec![];
    A.push(0);
    A.push(N);
    let mut Aused = vec![false; N];
    while A.len() < M + 1 {
        let N_i32 = N as i32;
        let a = rng.gen_range(1i32..=N_i32 - 1) as usize;
        if !Aused[a] {
            A.push(a);
            Aused[a] = true;
        }
    }
    A.sort();
    for i in 0..M {
        G.push(A[i + 1] - A[i]);
    }

    let mut range = vec![];
    let mut xy = vec![];
    for _ in 0..N {
        let x = rng.gen_range(0i32..=10000);
        let y = rng.gen_range(0i32..=10000);
        let W_i = W as i32;
        let w = rng.gen_range(0i32..=W_i);
        let dx = rng.gen_range(0i32..=w);
        let dy = rng.gen_range(0i32..=w);
        let mut rx = x + dx;
        let mut ry = y + dy;
        let mut lx = rx - w;
        let mut ly = ry - w;
        //0以上10000以下の範囲に収まるようにする
        lx = lx.max(0).min(10000);
        rx = rx.max(0).min(10000);
        ly = ly.max(0).min(10000);
        ry = ry.max(0).min(10000);
        let lx_u = lx as usize;
        let rx_u = rx as usize;
        let ly_u = ly as usize;
        let ry_u = ry as usize;
        let x_u = x as usize;
        let y_u = y as usize;
        range.push((lx_u, rx_u, ly_u, ry_u));
        xy.push((x_u, y_u));
    }
    Input {
        N,
        M,
        Q,
        L,
        W,
        G,
        range,
        xy,
    }
}

pub fn compute_score(input: &Input, outs: &Vec<String>) -> (i64, String) {
    let (mut score, err, _) = compute_score_details(input, &outs);
    if err.len() > 0 {
        score = 0;
    }
    (score, err)
}

pub fn compute_score_details(
    input: &Input,
    outs: &Vec<String>,
) -> (i64, String, (Vec<Vec<usize>>, Vec<Vec<(usize, usize)>>)) {
    let dist = build_dist_matrix(input);
    //各グループについての回答をM回受け取り、バリデーションとスコアの計算を行う
    let mut city_group = vec![-1; input.N];
    let mut city_id = vec![-1; input.N];
    let mut line_ptr = 0;
    let mut score = 0 as i64;
    let mut groups = vec![];
    let mut edges = vec![];
    for g in 0..(input.M) {
        groups.push(vec![]);
        edges.push(vec![]);
        let line = &outs[line_ptr];
        line_ptr += 1;
        let mut tokens = line.split_whitespace();
        for k in 0..input.G[g] {
            let i = read(tokens.next(), 0..input.N);
            let i = match i {
                Ok(i) => i,
                Err(err) => return (0, err, (vec![], vec![])),
            };
            if city_group[i] != -1 {
                return (
                    0,
                    format!("Item {} appears multiple times.", i),
                    (vec![], vec![]),
                );
            }
            city_group[i] = g as i32;
            city_id[i] = k as i32;
            groups[g].push(i);
        }
        if tokens.next().is_some() {
            return (
                0,
                format!("Illegal output format: {}", line),
                (vec![], vec![]),
            );
        }

        let mut dsu = Dsu::new(input.G[g]);
        for _ in 0..(input.G[g] - 1) {
            let line = &outs[line_ptr];
            line_ptr += 1;
            let mut tokens = line.split_whitespace();
            let i = read(tokens.next(), 0..input.N);
            let i = match i {
                Ok(i) => i,
                Err(err) => return (0, err, (vec![], vec![])),
            };
            let j = read(tokens.next(), 0..input.N);
            let j = match j {
                Ok(j) => j,
                Err(err) => return (0, err, (vec![], vec![])),
            };
            if tokens.next().is_some() {
                return (
                    0,
                    format!("Illegal output format: {}", line),
                    (vec![], vec![]),
                );
            }
            if city_group[i] != g as i32 || city_group[j] != g as i32 {
                return (0, format!("Invalid edge: {} {}", i, j), (vec![], vec![]));
            }
            let id_i = city_id[i] as usize;
            let id_j = city_id[j] as usize;
            if dsu.same(id_i, id_j) {
                return (0, format!("Invalid edge: {} {}", i, j), (vec![], vec![]));
            }
            dsu.merge(id_i, id_j);
            score += dist[i][j] as i64;
            edges[g].push((i, j));
        }
    }
    if line_ptr != outs.len() {
        return (0, "Too many outputs".to_owned(), (vec![], vec![]));
    }
    (score, "".to_owned(), (groups, edges))
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

pub fn build_dist_matrix(input: &Input) -> Vec<Vec<usize>> {
    let mut dist = vec![vec![0; input.N]; input.N];
    for i in 0..input.N {
        for j in 0..input.N {
            let mut d = ((input.xy[i].0 as i64 - input.xy[j].0 as i64).pow(2)
                + (input.xy[i].1 as i64 - input.xy[j].1 as i64).pow(2))
                as f64;
            d = d.sqrt();
            //切り捨てて整数とする
            d = d.floor();
            dist[i][j] = d as usize;
        }
    }
    dist
}

pub fn get_query_from_line_and_validate(input: &Input, line: &String) -> (Vec<usize>, String) {
    let mut tokens = line.split_whitespace();
    let c = read(tokens.next(), '!'..='?');
    let c = match c {
        Ok(c) => c,
        Err(err) => return (vec![], err),
    };
    if c != '?' {
        return (vec![], format!("Unknown char: {}", c));
    }
    let l = read(tokens.next(), 2..=input.L);
    let l = match l {
        Ok(l) => l,
        Err(err) => return (vec![], err),
    };
    let mut query = vec![];
    let mut used = vec![false; input.N];
    for _ in 0..l {
        let i = read(tokens.next(), 0..input.N);
        let i = match i {
            Ok(i) => i,
            Err(err) => return (vec![], err),
        };
        if !used[i].setmax(true) {
            return (vec![], format!("Item {} appears multiple times.", i));
        }
        query.push(i);
    }
    if tokens.next().is_some() {
        return (vec![], format!("Illegal output format: {}", line));
    }
    query.sort();
    (query, "".to_owned())
}

pub fn do_query(
    input: &Input,
    line: &String,
    dist: &Vec<Vec<usize>>,
) -> (Vec<(usize, usize)>, String, Vec<usize>) {
    let (query, err) = get_query_from_line_and_validate(input, line);
    if err.len() > 0 {
        return (vec![], err, vec![]);
    }

    let mut edges_candidate = vec![];
    for i in 0..query.len() {
        for j in i + 1..query.len() {
            edges_candidate.push((dist[query[i]][query[j]], i, j));
        }
    }
    edges_candidate.sort();
    let mut dsu = Dsu::new(query.len());
    let mut edges = vec![];
    for (_, i, j) in edges_candidate {
        if dsu.same(i, j) {
            continue;
        }
        dsu.merge(i, j);
        edges.push((query[i], query[j]));
    }
    edges.sort();
    (edges, "".to_owned(), query)
}

fn rect(x: i32, y: i32, w: i32, h: i32, fill: &str, fill_opacity: f32) -> Rectangle {
    Rectangle::new()
        .set("x", x)
        .set("y", y)
        .set("width", w)
        .set("height", h)
        .set("fill", fill)
        .set("fill-opacity", fill_opacity)
}

pub fn vertex_group(
    i: usize,
    x: usize,
    y: usize,
    true_x: usize,
    true_y: usize,
    lx: usize,
    rx: usize,
    ly: usize,
    ry: usize,
    do_highlight: bool,
    group_id: i32,
) -> Group {
    let W = 800;
    Group::new()
        .add(
            Circle::new()
                .set("cx", x * W / 10000)
                .set("cy", y * W / 10000)
                .set("r", if do_highlight { 4 } else { 3 })
                .set("fill", if do_highlight { "red" } else { "black" })
                .set("onclick", format!("set_s({})", i))
                .set("class", "vis_circle")
                .add(Title::new().add(Text::new(
                    if group_id == -1 {
                        format!("vertex {}, (x, y) = ({}, {}), (lx, rx, ly, ry) = ({}, {}, {}, {})", i, true_x, true_y, lx, rx, ly, ry)
                    } else {
                        format!("group {}, vertex {}, (x, y) = ({}, {}), (lx, rx, ly, ry) = ({}, {}, {}, {})", group_id, i, true_x, true_y, lx, rx, ly, ry)
                    }
                )))
        )
        .add(
            rect(
                (lx * W / 10000) as i32,
                (ly * W / 10000) as i32,
                ((rx - lx) * W / 10000) as i32,
                ((ry - ly) * W / 10000) as i32,
                "blue",
                0.0
            )
            .set("class", "vis_rect")
            .set("pointer-events", "none")
        )
}

pub fn edge_group(
    i: usize,
    j: usize,
    x: usize,
    y: usize,
    x2: usize,
    y2: usize,
    dist: usize,
    do_highlight: bool,
    group_id: i32,
) -> Group {
    let W = 800;
    Group::new().add(
        Line::new()
            .set("x1", x * W / 10000)
            .set("y1", y * W / 10000)
            .set("x2", x2 * W / 10000)
            .set("y2", y2 * W / 10000)
            .set("stroke", if do_highlight { "red" } else { "black" })
            .set("stroke-width", if do_highlight { 3 } else { 2 })
            .set("onclick", format!("set_s({})", i))
            .add(Title::new().add(Text::new(if group_id == -1 {
                format!("edge (a, b) = ({}, {}), dist={}", i, j, dist)
            } else {
                format!(
                    "group {}, edge (a, b) = ({}, {}), dist={}",
                    group_id, i, j, dist
                )
            }))),
    )
}

pub fn vis_default(input: &Input, out: &Output) -> (i64, String, String) {
    let (mut score, err, svg) = vis(input, out, out.queries.len(), "true");
    if err.len() > 0 {
        score = 0;
    }
    (score, err, svg)
}

pub fn vis(input: &Input, out: &Output, t: usize, coordinate: &str) -> (i64, String, String) {
    let W = 800;
    let mut doc = svg::Document::new()
        .set("id", "vis")
        .set("viewBox", (-5, -5, W + 10, W + 10))
        .set("width", W + 10)
        .set("height", W + 10);
    doc = doc.add(rect(-5, -5, (W + 10) as i32, (W + 10) as i32, "white", 1.0));
    doc = doc.add(Style::new(format!(
        "text {{text-anchor: middle;dominant-baseline: central;}}
        .vis_circle:hover ~ .vis_rect {{fill-opacity: 0.2;}}
        .answer_group:hover circle {{fill: red;r: 4;}}
        .answer_group:hover line {{stroke: red;stroke-width: 3;}}"
    )));
    let mut x_input = vec![];
    let mut y_input = vec![];
    let mut x_true = vec![];
    let mut y_true = vec![];
    for i in 0..input.N {
        let (lx, rx, ly, ry) = input.range[i];
        x_input.push((lx + rx) / 2 as usize);
        y_input.push((ly + ry) / 2 as usize);
        x_true.push(input.xy[i].0);
        y_true.push(input.xy[i].1);
    }

    let mut x = &x_input;
    let mut y = &y_input;
    if coordinate == "true" {
        x = &x_true;
        y = &y_true;
    }

    let dist = build_dist_matrix(&input);

    let query_size = out.queries.len();

    //validation
    if query_size > input.Q {
        return (0, "Too many queries".to_owned(), String::new());
    }
    for i in 0..query_size {
        let (_, err) = get_query_from_line_and_validate(&input, &out.queries[i]);
        if err.len() > 0 {
            return (0, err, String::new());
        }
    }
    let mut score = 0;
    let mut groups = vec![];
    let mut edges = vec![];
    if out.outputs.len() > 0 {
        let (score_tmp, err, groups_and_edges) = compute_score_details(&input, &out.outputs);
        if err.len() > 0 {
            return (0, err, String::new());
        }
        score = score_tmp;
        groups = groups_and_edges.0;
        edges = groups_and_edges.1;
    }

    if t < query_size {
        //query
        let line = &out.queries[t];
        let (edges, err, query) = do_query(&input, &line, &dist);
        if err.len() > 0 {
            return (0, err, String::new());
        }
        for (i, j) in edges {
            doc = doc.add(edge_group(
                i, j, x[i], y[i], x[j], y[j], dist[i][j], true, -1,
            ));
        }
        let mut is_in = vec![false; input.N];
        for i in query {
            is_in[i] = true;
        }
        for i in 0..input.N {
            doc = doc.add(vertex_group(
                i,
                x[i],
                y[i],
                x_true[i],
                y_true[i],
                input.range[i].0,
                input.range[i].1,
                input.range[i].2,
                input.range[i].3,
                is_in[i],
                -1,
            ));
        }
    } else if out.outputs.len() == 0 {
        //default output
        for i in 0..input.N {
            doc = doc.add(vertex_group(
                i,
                x[i],
                y[i],
                x_true[i],
                y_true[i],
                input.range[i].0,
                input.range[i].1,
                input.range[i].2,
                input.range[i].3,
                false,
                -1,
            ));
        }
    } else {
        //output
        for group_id in 0..groups.len() {
            let group = &groups[group_id];
            let edges = &edges[group_id];
            let mut answer_group = Group::new().set("class", "answer_group");
            for (i, j) in edges {
                answer_group = answer_group.add(edge_group(
                    *i,
                    *j,
                    x[*i],
                    y[*i],
                    x[*j],
                    y[*j],
                    dist[*i][*j],
                    false,
                    group_id as i32,
                ));
            }
            for i in group {
                answer_group = answer_group.add(vertex_group(
                    *i,
                    x[*i],
                    y[*i],
                    x_true[*i],
                    y_true[*i],
                    input.range[*i].0,
                    input.range[*i].1,
                    input.range[*i].2,
                    input.range[*i].3,
                    false,
                    group_id as i32,
                ));
            }
            doc = doc.add(answer_group);
        }
    }

    (score, "".to_owned(), doc.to_string())
}

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
        if v.len() == 0 {
            continue;
        }
        return Ok(v.to_owned());
    }
}

pub fn exec(p: &mut std::process::Child, local: bool) -> Result<i64, String> {
    let mut input = String::new();
    std::io::stdin().read_to_string(&mut input).unwrap();
    let input = parse_input(&input);
    let mut stdin = std::io::BufWriter::new(p.stdin.take().unwrap());
    let mut stdout = std::io::BufReader::new(p.stdout.take().unwrap());
    let _ = writeln!(
        stdin,
        "{} {} {} {} {}",
        input.N, input.M, input.Q, input.L, input.W
    );
    let _ = writeln!(stdin, "{}", input.G.iter().join(" "));
    for i in 0..input.N {
        let _ = writeln!(
            stdin,
            "{} {} {} {}",
            input.range[i].0, input.range[i].1, input.range[i].2, input.range[i].3
        );
    }
    let _ = stdin.flush();
    //真の座標についてユークリッド距離行列を作成する
    let dist = build_dist_matrix(&input);
    let mut score = 0;
    for q in 0..(input.Q + 1) {
        let line = read_line(&mut stdout, local)?;
        let mut tokens = line.split_whitespace();
        let c = read(tokens.next(), '!'..='?')?;
        if c != '!' && c != '?' {
            return Err(format!("Unknown char: {}", c));
        }
        if c == '?' {
            if q == input.Q {
                return Err(format!("Too many queries: {}", line));
            }
            let (edges, err, _) = do_query(&input, &line, &dist);
            if err.len() > 0 {
                return Err(err);
            }
            //辺の集合を出力
            for (i, j) in edges {
                let _ = writeln!(stdin, "{} {}", i, j);
            }
            let _ = stdin.flush();
        } else {
            if tokens.next().is_some() {
                return Err(format!("Illegal output format: {}", line));
            }
            let mut outputs = vec![];
            for _ in 0..input.N {
                let line = read_line(&mut stdout, local)?;
                outputs.push(line);
            }
            let (score_tmp, err, _) = compute_score_details(&input, &outputs);
            if err.len() > 0 {
                return Err(err);
            }
            score = score_tmp;
            break;
        }
    }
    if read_line(&mut stdout, local).is_ok() {
        return Err("Too many output".to_owned());
    }
    Ok(score as i64)
}
