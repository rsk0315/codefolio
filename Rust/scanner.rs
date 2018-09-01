fn scan<T: std::str::FromStr>() -> T {
    let mut s = String::new();
    std::io::stdin().read_line(&mut s).ok();
    s.trim().parse().ok().unwrap()
}

fn scan_split<T: std::str::FromStr>() -> Vec<T> {
    let mut s = String::new();
    std::io::stdin().read_line(&mut s).ok();
    s.trim().split_whitespace()
        .map(|t| t.parse().ok().unwrap()).collect()
}

fn scan_split_with<T: std::str::FromStr>(ch: char) -> Vec<T> {
    let mut s = String::new();
    std::io::stdin().read_line(&mut s).ok();
    s.trim().split(ch)
        .map(|t| t.parse().ok().unwrap()).collect()
}

fn scan_lines<T: std::str::FromStr>(n: usize) -> Vec<T> {
    (0..n).map(|_| scan::<T>()).collect()
}

macro_rules! scan_as_tuple {
    ( $t:ty ) => {
        (scan::<$t>(),)
    };
    ( $( $t:ty ),* ) => {
        {
            let mut s = String::new();
            std::io::stdin().read_line(&mut s).ok();
            let v = s.trim().split_whitespace();
            let mut it = v;
            let ret = ( $(it.next().unwrap().parse::<$t>().ok().unwrap()),* );
            assert!(it.next() == None, "not all tokens consumed");
            ret
        }
    };
}
