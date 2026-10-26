import type { CSSProperties } from "react";

import p from "../assets/pecas/peaoPreto.png";
import r from "../assets/pecas/torrePreta.png";
import n from "../assets/pecas/cavaloPreto.png";
import b from "../assets/pecas/bispoPreto.png";
import q from "../assets/pecas/rainhaPreta.png";
import k from "../assets/pecas/reiPreto.png";

import P from "../assets/pecas/peaoBranco.png";
import R from "../assets/pecas/torreBranca.png";
import N from "../assets/pecas/cavaloBranco.png";
import B from "../assets/pecas/bispoBranco.png";
import Q from "../assets/pecas/rainhaBranca.png";
import K from "../assets/pecas/reiBranco.png";

const PECAS = [
    p, r, n, b, q, k,
    P, R, N, B, Q, K
];

const QUANTIDADE = 12;               
const TAMANHOS = [48, 64, 96];       

const FundoPecas = () => {
    return (
        <div className="fundo-pecas" aria-hidden="true">
            {Array.from({ length: QUANTIDADE }, (_, i) => {

                const ficha = {
                    "--x": `${(i * 37) % 100}%`,                 // posição horizontal
                    "--tamanho": `${TAMANHOS[i % TAMANHOS.length]}px`,
                    "--duracao": `${25 + ((i * 7) % 20)}s`,      // de 25s a 44s: lento
                    "--atraso": `-${(i * 5) % 30}s`,             // negativo: já começa no meio do caminho
                } as CSSProperties;

                return (
                    <img
                        key={i}
                        src={PECAS[i % PECAS.length]}
                        className="peca-fundo"
                        style={ficha}
                        alt=""
                        draggable={false}
                    />
                );
            })}
        </div>
    );
};

export default FundoPecas;