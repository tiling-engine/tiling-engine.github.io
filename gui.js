
const grid = document.getElementById("grid");
const size = 64;

const cells = Array.from(
    { length: size },
    () => Array(size).fill(false)
);

const res = document.getElementById("count");

for (let y = 0; y < size; y++) {
    for (let x = 0; x < size; x++) {
        const cell = document.createElement("div");
        cell.className = "cell";

        cell.addEventListener("click", () => {
            cells[y][x] = !cells[y][x];
            cell.style.background = cells[y][x] ? "#4ade80" : "#333";
            res.innerHTML = count(cells);
        });

        grid.appendChild(cell);
    }
}

function packGrid(cells) {
    const rows = new BigUint64Array(64);
    for (let y = 0; y < 64; y++) {
        let bits = 0n;

        for (let x = 0; x < 64; x++) {
            if (cells[y][x]) {
                bits |= 1n << BigInt(x);
            }
        }
        rows[y] = bits;
    }
    return rows;
}


function count(cells) {
    const rows = packGrid(cells);
    const bytes = rows.byteLength;

    const ptr = Module._malloc(bytes);

    if (!ptr) {
        throw new Error("WebAssembly memory allocation failed");
    }

    try {
        const offset = ptr / 8;

        for (let y = 0; y < 64; y++) {
            Module.HEAPU64[offset + y] = rows[y];
        }

        const result = Module._countDomino(ptr);
        return result;
    } finally {
        Module._free(ptr);
    }
}
