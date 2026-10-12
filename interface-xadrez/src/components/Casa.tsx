import type { CSSProperties } from "react";
import type { CasaProps } from "../interfaces/CasaProps";

const Casa = ({ row, col, peca, selecionada, onSelecionar, movimentoPossivel }: CasaProps) => {

    // o TypeScript não conhece variáveis CSS, então precisa do cast
    const posicao = { "--col": col, "--lin": row } as CSSProperties;

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
            style={posicao}
            data-row={row}
            data-col={col}
            onClick={selecionarCasa}
        >
            {peca && (
                <img
                    src={peca.img}
                    className="peca"
                    data-tipo={peca.tipo}
                    alt={peca.tipo}
                    draggable={false}
                />
            )}
        </div>
    );
};

export default Casa;
