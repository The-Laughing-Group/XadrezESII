import type { CSSProperties } from "react";

import type { Peca } from "../interfaces/Peca";

export interface CasaProps {
    row: number;
    col: number;
    peca: Peca | null;
    selecionada: boolean;
    onSelecionar: (row: number, col: number) => void;
    movimentoPossivel: boolean;
    podeSelecionar: boolean | undefined;
}

const Casa = ({ row, 
                col, 
                peca, 
                selecionada, 
                onSelecionar, 
                movimentoPossivel, 
                podeSelecionar }: CasaProps) => {

    // o TypeScript não conhece variáveis CSS, então precisa do cast
    const estilo = { "--col": col, "--lin": row } as CSSProperties;

    const selecionarCasa = () => {
        onSelecionar(row, col);
    };

    return (
        <div
            className={
                `casa
                ${selecionada === true ? 'casa-selecionada' 
                : movimentoPossivel === true && (row + col) % 2 === 0? 'movimento-possivel-branco' 
                : movimentoPossivel === true && (row + col) % 2 !== 0? 'movimento-possivel-preto' 
                : (row + col) % 2 === 0 ? 'casa-branca' : 'casa-preta'}
                `
            }
            style={estilo}
            data-row={row}
            data-col={col}
        >
            <div className="casa-area" onClick={selecionarCasa} />
            { peca && (
                <img
                    src={peca.img}
                    className={`peca ${podeSelecionar ? "peca-selecionavel" : ""}`}
                    data-tipo={peca.tipo}
                    alt={peca.tipo}
                    draggable={false}
                    onClick={selecionarCasa}
                />
            )}
            
        </div>
    );
};

export default Casa;
