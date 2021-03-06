put {
    a[100] = p[0]
    putNATIVE()
}

read {
    readNATIVE()
    return a[100]
}

printLn {
    put(10)
}

toDig {
    return p[0] - '0'
}

printDig {
    put('0' + p[0])
}

putStr {
    put(a[0])
    put(a[1])
    put(a[2])
    put(a[3])
}

mul {
    a = p[0]
    b = p[1]
    result = 0
    while a {
        a = a - 1
        result = result + b
    }
    return result
}

g {
    a = p[0]
    b = p[1]
    while mul(a b) {
        a = a - 1
        b = b - 1
    }
    if a {
        return 1
    } else {
        return 0
    }
}

ge {
    a = p[0]
    b = p[1]
    while mul(a b) {
        a = a - 1
        b = b - 1
    }
    
    if b {
        return 0
    } else {
        return 1
    }
}

l {
    return g(p[1] p[0])
}

le {
    return ge(p[1] p[0])
}

div {
    a = p[0]
    b = p[1]
    result = 0
    while ge(a b) {
        a = a - b
        result = result + 1
    }
    return result
}

mod {
    return p[0] - mul(p[1] div(p[0] p[1]))
}

sqrt {
    r = 0
    while l(mul(r r) p[0]) {
        r = r + 1
    }
    return r
}

pow {
    a = p[0]
    n = p[1]

    r = 1
    while n {
        n = n - 1
        r = mul(r a)
    }

    return r
}

printInt {
    a = p[0]
    top = 100

    if a {
        while g(top a) {
            top = div(top 10)
        }

        while top {
            dv = div(a top)
            printDig(dv)

            a = a - mul(dv top)
            top = div(top 10)
        }
    } else {
        printDig(0)
    }
}

main {
    a = toDig(read())
    n = toDig(read())
    printInt(pow(a n))
    printLn()
}
