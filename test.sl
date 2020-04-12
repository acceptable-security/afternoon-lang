let a: number = 1 + 2;
let b: string = "a" + "b";
let c: list[:number] = [ 1, 2 ];
let d: list[:number] = [ 2, 3 ] + [ 4 ];

let e :function[:number, :list[:number]] = function (a: number) :list[:number] {
    return [ a + 1 ] + d;
};

let f :list[:number] = e(a);
return f;