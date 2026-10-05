
import type { Peca } from "../interfaces/Peca";

interface CasaProps {
    row: number;
    col: number;
    peca: Peca | null;
}

const Casa = ({ row, col, peca }: CasaProps) => {
    return (
        <div
            className={`casa ${(row + col) % 2 === 0 ? 'casa-branca' : 'casa-preta'}`}
            data-row={row}
            data-col={col}
        >
            {peca && (
                <img
                    src={peca.img}
                    className="peca"
                    alt={peca.tipo}
                />
            )}
        </div>
    );
};

export default Casa;