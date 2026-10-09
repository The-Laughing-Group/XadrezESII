import type { CSSProperties } from "react";
import type { Peca } from "../interfaces/Peca";

interface CasaProps {
    row: number;
    col: number;
    peca: Peca | null;
}

const Casa = ({ row, col, peca }: CasaProps) => {
    // o TypeScript não conhece variáveis CSS, então precisa do cast
    const posicao = { "--col": col, "--lin": row } as CSSProperties;

    return (
        <div
            className={`casa ${(row + col) % 2 === 0 ? "casa-branca" : "casa-preta"}`}
            style={posicao}
            data-row={row}
            data-col={col}
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