import Casa from "./Casa";
import type { Peca, TipoPeca } from "../interfaces/Peca";

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
import type { Partida } from "../hooks/usePartida";

import { useState } from "react";

const criarPeca = (tipo: TipoPeca): Peca => {

    const imagens = {p, r, n, b, q, k, P, R, N, B, Q, K};

    return {
        tipo,
        img: imagens[tipo]
    };
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

type Posicao = { 
    row: number; 
    col: number; 
};

const FEN_PARA_TIPO: Record<string, TipoPeca> = {
    p: "p", r: "r", n: "n", b: "b", q: "q", k: "k",
    P: "P", R: "R", N: "N", B: "B", Q: "Q", K: "K",
};

const OPCOES_DE_PROMOCAO = [
    ["q", "Dama"],
    ["r", "Torre"],
    ["b", "Bispo"],
    ["n", "Cavalo"],
] as const;

const historicoMovimentos: (string)[] = [];

const Tabuleiro = ({ partida }: { partida: Partida }) => {

    const {pronta, tabuleiro, vezDasBrancas, lancesDe, tentarLance } = partida;

    const [casaSelecionada, setCasaSelecionada] = useState<Posicao| null>(null);

    const [movimentosPossiveis, setMovimentosPossiveis] = useState<Posicao[]>([]);

    const [promocao, setPromocao] = useState<{origem: Posicao; destino: Posicao} | null>(null);

    const casaEstaSelecionada = (row: number, col: number): boolean => {
        return casaSelecionada?.row === row && casaSelecionada?.col === col;
    };

    const eMovimentoPossivel = (row: number, col: number): boolean => {
        return movimentosPossiveis.some((posicao) => posicao.row === row && posicao.col === col);
    };

    const podeSelecionar = (row: number, col: number) => {
        const peca = tabuleiro[row]?.[col];
        if(peca === ".") return false;
        const branca = peca === peca?.toUpperCase();
        return vezDasBrancas ? branca : !branca;
    }

    const addMovimentoHistorico = (row: number, col: number, promocao = ""):void => {
        const destino = posicaoToUci(row, col);
        if(!casaSelecionada) return;
        const peca = tabuleiro[casaSelecionada.row][casaSelecionada.col];
        if(peca.toUpperCase() === "p".toUpperCase()) {
            historicoMovimentos.push(destino+promocao);
        }
        else {
            historicoMovimentos.push(peca.toUpperCase() + destino + promocao);
        }
    };

    const limparSelecao = () => {
        setCasaSelecionada(null); 
        setMovimentosPossiveis([]);
        setPromocao(null);
    };

    const escolherPromocao = (letra: string) => {
        if (!promocao) return;
        if(tentarLance(posicaoToUci(promocao.origem.row, 
                                 promocao.origem.col) + 
                    posicaoToUci(promocao.destino.row, 
                                 promocao.destino.col) + 
                    letra)) addMovimentoHistorico(promocao.destino.row, promocao.destino.col, letra.toUpperCase());
        limparSelecao();
    };

    const selecionarCasa = (row: number, col: number) => {
        if(promocao) return;

        if(casaEstaSelecionada(row, col)) {
            limparSelecao();
            return;
        };

        // Realiza movimento
        if(casaSelecionada && eMovimentoPossivel(row, col)) {
            const uci = posicaoToUci(casaSelecionada.row, casaSelecionada.col);
            const movimentos = lancesDe(uci).filter(movimento => movimento.slice(2, 4) === posicaoToUci(row, col));

            if(movimentos.some(movimento => movimento.length === 5)) {
                setPromocao({origem: { row: casaSelecionada.row, 
                                       col: casaSelecionada.col }, 
                            destino: { row: row, col: col }});
                return;
            }

            if(tentarLance(posicaoToUci(casaSelecionada.row, casaSelecionada.col) + posicaoToUci(row, col))) addMovimentoHistorico(row, col);
            limparSelecao();
            return;
        }

        if(podeSelecionar(row, col)) {
            setCasaSelecionada({ row, col });

            const uci = posicaoToUci(row, col);
            
            setMovimentosPossiveis(
                lancesDe(uci).map((possivelMovimento) =>
                    uciToPosicao(possivelMovimento)
                ) || []
            );
            setPromocao(null);
            return;
        }
    }

    if(!pronta) return;

    return (
        <>
            <div className="">
                <div>Vez das {vezDasBrancas ? "brancas" : "pretas"}</div>
                { promocao && (
                        <>
                            <span>Promover para:</span>
                            {OPCOES_DE_PROMOCAO.map(([tipo, nome]) => (
                                <button key={tipo} onClick={() => escolherPromocao(tipo)}>
                                    {nome}
                                </button>
                            ))}
                            <button onClick={limparSelecao}>Cancelar</button>
                        </>
                    )}
            
                <div id="tabuleiro">
                    {tabuleiro.flatMap((linha, row) =>
                        Array.from(linha).map((tipo, col) => {
                            return (
                                <Casa
                                    key={`${row}-${col}`}
                                    row={row}
                                    col={col}
                                    peca={tipo === "." ? null : criarPeca(FEN_PARA_TIPO[tipo])}
                                    selecionada={casaEstaSelecionada(row,col)}
                                    onSelecionar={selecionarCasa}
                                    movimentoPossivel={eMovimentoPossivel(row, col)}
                                    podeSelecionar={podeSelecionar(row,col)}
                                />
                            );
                        })
                    )}
                </div>
            </div>

            <div id="historico-movimentos">
                <div className="coluna-movimentos">
                    <h3>Brancas</h3>

                    {historicoMovimentos
                        .filter((_, index) => index % 2 === 0)
                        .map((movimento, index) => (
                            <p key={index}>
                                {index + 1}. {movimento}
                            </p>
                        ))}
                </div>

                <div className="coluna-movimentos">
                    <h3>Pretas</h3>

                    {historicoMovimentos
                        .filter((_, index) => index % 2 !== 0)
                        .map((movimento, index) => (
                            <p key={index + 1}>
                                {index + 1}. {movimento}
                            </p>
                        ))}
                </div>
            </div>
        </>
    );
};

export default Tabuleiro;
