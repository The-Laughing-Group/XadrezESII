import type { CSSProperties } from "react";
import type { CasaProps } from "../interfaces/CasaProps";

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
                : movimentoPossivel === true? 'movimento-possivel' 
                : (row + col) % 2 === 0 ? 'casa-branca' : 'casa-preta'}
                `
            }
            style={estilo}
            data-row={row}
            data-col={col}
            onClick={selecionarCasa}
        >
            {peca && (
                <img
                    src={peca.img}
                    className={`peca ${podeSelecionar ? "peca-selecionavel" : ""}`}
                    data-tipo={peca.tipo}
                    alt={peca.tipo}
                    draggable={false}
                />
            )}
        </div>
    );
};

export default Casa;
