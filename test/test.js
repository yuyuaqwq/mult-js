function add(a, b, println) {
    println("a:", a, "b:", b);
    a = a + b;

    function bbb(c, d, e, println) {
        println("c:", c, "d:", d, "e:", e);
        let f = c + d + e;
        return f;
    }
    // println("add call");
    // println(a);
    let c = bbb(a, a, b, println);
    return c;
}

// println("hello world!");

let a = 1;
let b = add(666, a, println);
println("b", b);