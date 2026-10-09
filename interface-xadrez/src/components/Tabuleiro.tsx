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

import { useState } from "react";
import { useJogo } from "../hooks/useJogo";

const criarPeca = (tipo: TipoPeca): Peca => {

    const imagens = {p, t, c, b, q, k, P, T, C, B, Q, K};

    return {
        tipo,
        img: imagens[tipo]
    };
};

type Posicao = { 
    row: number; 
    col: number; 
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

    const [tabuleiro, setTabuleiro] = useState<(TipoPeca | null)[][]>(
        () => tabuleiroInicial.map((linha) => [...linha])
    );

    const [casaSelecionada, setCasaSelecionada] = useState<Posicao| null>(null);

    const [movimentosPossiveis, setMovimentosPossiveis] = useState<Posicao[]>([]);

    const jogo = useJogo();

    const casaEstaSelecionada = (row: number, col: number) => {
        return casaSelecionada?.row === row && casaSelecionada?.col === col;
    };

    const eMovimentoPossivel = (row: number, col: number) => {
        return movimentosPossiveis.some((posicao) => posicao.row === row && posicao.col === col);
    };

    const posicaoToUci = (row: number, col: number):string => {
        const coluna = String.fromCharCode("a".charCodeAt(0) + col);
        const linha = 8 - row;
        return `${coluna}${linha}`;
    };

    const uciToPosicao = (uci: string): Posicao => {
        const destino = uci.substring(2, 4);

        const col = destino.charCodeAt(0) - "a".charCodeAt(0);
        const row = 8 - Number(destino[1]);

        return { row, col };
    };

    const selecionarCasa = (row: number, col: number) => {
        const peca = tabuleiro[row][col];
        
        if(casaEstaSelecionada(row, col)) {
            setCasaSelecionada(null); 
            setMovimentosPossiveis([]);
            return;
        }

        if(peca && !eMovimentoPossivel(row, col)) {
            setCasaSelecionada({ row, col });

            const uci = posicaoToUci(row,col);
            
            setMovimentosPossiveis(
                jogo?.lancesDe(uci).map((possivelMovimento) =>
                    uciToPosicao(possivelMovimento)
                ) || []
            );
            return;
        }

        if(casaSelecionada && eMovimentoPossivel(row, col)) {
            const origem = casaSelecionada;

            setTabuleiro((tabuleiroAtual) => {
                const novoTabuleiro = tabuleiroAtual.map(
                    (linha) => [...linha]
                );

                novoTabuleiro[row][col] = novoTabuleiro[origem.row][origem.col];
                novoTabuleiro[origem.row][origem.col] = null;

                return novoTabuleiro;
            });

            setCasaSelecionada(null);
            setMovimentosPossiveis([]);
            return;
        }
    }

    return (
        <div id="tabuleiro">

            {tabuleiro.map((linha, row) =>
                linha.map((tipo, col) => (
                    <Casa
                        key={`${row}-${col}`}
                        row={row}
                        col={col}
                        peca={tipo ? criarPeca(tipo) : null}
                        selecionada={casaEstaSelecionada(row,col)}
                        onSelecionar={selecionarCasa}
                        movimentoPossivel={eMovimentoPossivel(row, col)}
                    />

                ))
            )}

        </div>
    );
};

export default Tabuleiro;
