export type TipoPeca = 'p' | 'r' | 'n' | 'b' | 'q' | 'k' | 'P' | 'R' | 'N' | 'B' | 'Q' | 'K'

export interface Peca {
    tipo: TipoPeca;
    img: string;
}