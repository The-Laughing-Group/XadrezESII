// Símbolo de cada tipo de peça. Usamos os símbolos "cheios" para as duas cores e
// diferenciamos pela cor do texto (classes peca-branca / peca-preta no index.css).
// O \uFE0E força o desenho em texto (sem virar emoji colorido, o que acontece com o peão).

interface CasaProps {
    row: number
    col: number
}

const Casa = ({row, col} : CasaProps) => {
    return (
        <>
            <div className={`casa ${(row + col) % 2 === 0 ? 'casa-branca' : 'casa-preta'}`} 
                data-row={`${row}`} 
                data-col={`${col}`} >
            </div>
        </>
    )
}
export default Casa


//EXEMPLO de estrutura

// const SIMBOLOS: Record<string, string> = {
//     k: '♚', q: '♛', r: '♜', b: '♝', n: '♞', p: '♟\uFE0E',
// }

// interface CasaProps {
//     row: number
//     col: number
//     peca: string            // letra do FEN (maiúscula = branca), ou '.' se a casa está vazia
//     selecionada: boolean
//     destino: boolean        // é um destino possível da peça selecionada?
//     aoClicar: () => void
// }

// const Casa = ({ row, col, peca, selecionada, destino, aoClicar }: CasaProps) => {
//     const simbolo = SIMBOLOS[peca.toLowerCase()]
//     const branca = peca !== peca.toLowerCase()   // letra maiúscula = peça branca

//     const classes = ['casa', (row + col) % 2 === 0 ? 'casa-branca' : 'casa-preta']
//     if (selecionada) classes.push('casa-selecionada')
//     if (destino) classes.push(simbolo ? 'casa-captura' : 'casa-destino')   // destino com peça = captura

//     return (
//         <div className={classes.join(' ')} data-row={row} data-col={col} onClick={aoClicar}>
//             {simbolo && <span className={`peca ${branca ? 'peca-branca' : 'peca-preta'}`}>{simbolo}</span>}
//         </div>
//     )
// }
// export default Casa
