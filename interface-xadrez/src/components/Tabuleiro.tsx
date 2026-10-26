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

import config from "../assets/icones/config.png"
import voltar from "../assets/icones/voltar.png"

import type { Partida } from "../hooks/usePartida";

import { useState } from "react";
import { useSom } from "../hooks/useSom";
import { Link } from "react-router-dom";

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
    ["q", "♛"],
    ["r", "♜"],
    ["b", "♝"],
    ["n", "♞"],
] as const;

const Tabuleiro = ({ partida, historicoMovimentos }: { partida: Partida, historicoMovimentos: (string)[] }) => {

    const {pronta, tabuleiro, vezDasBrancas, lancesDe, tentarLance } = partida;

    const [casaSelecionada, setCasaSelecionada] = useState<Posicao| null>(null);

    const [movimentosPossiveis, setMovimentosPossiveis] = useState<Posicao[]>([]);

    const [promocao, setPromocao] = useState<{origem: Posicao; destino: Posicao} | null>(null);

    const tocarMovimento = useSom(); 

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

    const addMovimentoHistorico = (row: number, col: number, { promocao = "", captura = "" }):void => {
        tocarMovimento();
        const destino = posicaoToUci(row, col);
        if(!casaSelecionada) return;
        const peca = tabuleiro[casaSelecionada.row][casaSelecionada.col];
        if(peca.toUpperCase() === "p".toUpperCase()) {
            historicoMovimentos.push(`${captura ? captura + "x" : ""}` + destino + promocao);
        }
        else {
            historicoMovimentos.push(peca.toUpperCase() + `${captura ? "x" : ""}` + destino + promocao);
        }
    };

    const limparSelecao = () => {
        setCasaSelecionada(null); 
        setMovimentosPossiveis([]);
        setPromocao(null);
    };

    const escolherPromocao = (letra: string) => {
        if (!promocao) return;

        const captura = tabuleiro[promocao.destino.row][promocao.destino.col] === "." ? "" : posicaoToUci(promocao.origem.row, promocao.origem.col).slice(0, 1);

        if(tentarLance(posicaoToUci(promocao.origem.row, 
                                 promocao.origem.col) + 
                       posicaoToUci(promocao.destino.row, 
                                    promocao.destino.col) + 
                       letra)) {
            addMovimentoHistorico(promocao.destino.row, 
                                 promocao.destino.col, 
                                 { promocao: letra.toUpperCase(),
                                   captura: captura
                                  });
        }
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

            const captura = tabuleiro[row][col] === "." ? "" : uci.slice(0, 1);

            if(tentarLance(posicaoToUci(casaSelecionada.row, casaSelecionada.col) + posicaoToUci(row, col))) {
                addMovimentoHistorico(row, col, { captura: captura });
            }
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
            <div className="barra-superior">
                <div className="barra-esq">
                    <Link className="botao-pixel botao-icone" to="/"
                        aria-label="Voltar" title="Voltar">
                        <img src={voltar} className="icone-botao" alt="" draggable={false} />
                    </Link>
                    <Link className="botao-pixel botao-icone" to="/configuracoes"
                        aria-label="Configurações" title="Configurações">
                        <img src={config} className="icone-botao" alt="" draggable={false} />
                    </Link>
                    <div className={`indicador-vez ${partida.vezDasBrancas ? "indicador-vez-branca" : "indicador-vez-preta"}`}>Vez das {partida.vezDasBrancas ? "brancas" : "pretas"}</div>
                </div>
                <div className="promocao">
                    { promocao && (
                            <>
                                {OPCOES_DE_PROMOCAO.map(([tipo, nome]) => (
                                    <button className="botao-pixel" key={tipo} onClick={() => escolherPromocao(tipo)}>
                                        {nome}
                                    </button>
                                ))}
                                <button className="botao-pixel cancelar" onClick={limparSelecao}>⨉</button>
                            </>
                    )}
                </div>   
            </div>
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
        </>
    );
};

export default Tabuleiro;
