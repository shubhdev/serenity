const TYPED_ARRAYS = [
    Uint8Array,
    Uint16Array,
    Uint32Array,
    Int8Array,
    Int16Array,
    Int32Array,
    Float32Array,
    Float64Array,
];

const BIGINT_TYPED_ARRAYS = [BigUint64Array, BigInt64Array];

test("length is 0", () => {
    TYPED_ARRAYS.forEach(T => {
        expect(T.prototype.values).toHaveLength(0);
    });

    BIGINT_TYPED_ARRAYS.forEach(T => {
        expect(T.prototype.values).toHaveLength(0);
    });
});

test("basic functionality", () => {
    TYPED_ARRAYS.forEach(T => {
        const a = new T([30, 40, 50]);
        const it = a.values();
        expect(it.next()).toEqual({ value: 30, done: false });
        expect(it.next()).toEqual({ value: 40, done: false });
        expect(it.next()).toEqual({ value: 50, done: false });
        expect(it.next()).toEqual({ value: undefined, done: true });
        expect(it.next()).toEqual({ value: undefined, done: true });
        expect(it.next()).toEqual({ value: undefined, done: true });
    });

    BIGINT_TYPED_ARRAYS.forEach(T => {
        const a = new T([30n, 40n, 50n]);
        const it = a.values();
        expect(it.next()).toEqual({ value: 30n, done: false });
        expect(it.next()).toEqual({ value: 40n, done: false });
        expect(it.next()).toEqual({ value: 50n, done: false });
        expect(it.next()).toEqual({ value: undefined, done: true });
        expect(it.next()).toEqual({ value: undefined, done: true });
        expect(it.next()).toEqual({ value: undefined, done: true });
    });
});
