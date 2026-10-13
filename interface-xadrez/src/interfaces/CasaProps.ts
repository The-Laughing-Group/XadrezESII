import type { Peca } from "./Peca";

export interface CasaProps {
    row: number;
    col: number;
    peca: Peca | null;
    selecionada: boolean;
    onSelecionar: (row: number, col: number) => void;
    movimentoPossivel: boolean;
}