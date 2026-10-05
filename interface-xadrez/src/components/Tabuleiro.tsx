import Casa from "./Casa";
import type { Peca, TipoPeca } from "../interfaces/Peca";

import p from "../assets/pecas/peaoPreto.png";
import t from "../assets/pecas/torrePreta.png";
import c from "../assets/pecas/cavaloPreto.png";
import b from "../assets/pecas/bispoPreto.png";
import q from "../assets/pecas/rainhaPreta.png";
import k from "../assets/pecas/reiPreto.png";

import P from "../assets/pecas/peaoBranco.png";
import T from "../assets/pecas/torreBranca.png";
import C from "../assets/pecas/cavaloBranco.png";
import B from "../assets/pecas/bispoBranco.png";
import Q from "../assets/pecas/rainhaBranca.png";
import K from "../assets/pecas/reiBranco.png";


const criarPeca = (tipo: TipoPeca): Peca => {

    const imagens = {p, t, c, b, q, k, P, T, C, B, Q, K};

    return {
        tipo,
        img: imagens[tipo]
    };
};


const tabuleiroInicial: (TipoPeca | null)[][] = [
    ["t", "c", "b", "q", "k", "b", "c", "t"],
    ["p", "p", "p", "p", "p", "p", "p", "p"],
    [null, null, null, null, null, null, null, null],
    [null, null, null, null, null, null, null, null],
    [null, null, null, null, null, null, null, null],
    [null, null, null, null, null, null, null, null],
    ["P", "P", "P", "P", "P", "P", "P", "P"],
    ["T", "C", "B", "Q", "K", "B", "C", "T"]
];


const Tabuleiro = () => {

    return (
        <div id="tabuleiro">

            {tabuleiroInicial.map((linha, row) =>
                linha.map((tipo, col) => (

                    <Casa
                        key={`${row}-${col}`}
                        row={row}
                        col={col}
                        peca={tipo ? criarPeca(tipo) : null}
                    />

                ))
            )}

        </div>
    );
};

export default Tabuleiro;