import Casa from "./Casa"
import { useJogo } from '../hooks/useJogo'

const Tabuleiro = () => {

    const jogo = useJogo()
    //funcoes:
    // jogo?.carregarFen
    // ...

    const t = [
        ["r", "n", "b", "q", "k", "b", "n", "r"],
        ["p", "p", "p", "p", "p", "p", "p", "p"],
        ["", "", "", "", "", "", "", ""],
        ["", "", "", "", "", "", "", ""],
        ["", "", "", "", "", "", "", ""],
        ["", "", "", "", "", "", "", ""],
        ["P", "P", "P", "P", "P", "P", "P", "P"],
        ["R", "N", "B", "Q", "K", "B", "N", "R"]
    ];

    return (
        <>
            <div id="tabuleiro">
                { t.map((linha, linhaIndex) => (
                    linha.map((casa, colunaIndex) => (
                        <Casa key={`${linhaIndex}-${colunaIndex}`} row={linhaIndex} col={colunaIndex}/>
                    ))
                )) }
            </div>
        </>
    )
}
export default Tabuleiro




// import { useState } from 'react'
// import Casa from './Casa'
// import { useJogo } from '../hooks/useJogo'

// const COLUNAS = 'abcdefgh'

// // linha 0 = 8ª fileira; coluna 0 = 'a'  ->  (6, 4) = "e2"
// const nomeDaCasa = (linha: number, coluna: number) => `${COLUNAS[coluna]}${8 - linha}`

// const Tabuleiro = () => {
//     const jogo = useJogo()
//     const [selecionada, setSelecionada] = useState<string | null>(null)
//     const [destinos, setDestinos] = useState<string[]>([])

//     if (!jogo) return <p>Carregando motor de regras...</p>

//     const limparSelecao = () => {
//         setSelecionada(null)
//         setDestinos([])
//     }

//     const aoClicar = (casa: string) => {
//         if (casa === selecionada) return limparSelecao()

//         // O motor decide o que é possível: o front só mostra o que ele devolve.
//         const lances = jogo.lancesDe(casa)               // ex.: ["e2e3", "e2e4"]
//         if (lances.length === 0) return limparSelecao()  // vazia, peça do adversário ou sem lances

//         setSelecionada(casa)
//         // Pega só a casa de destino (caracteres 3 e 4 do lance). Promoção gera 4 lances
//         // para o mesmo destino, então removemos os repetidos.
//         setDestinos([...new Set(lances.map(lance => lance.slice(2, 4)))])
//     }

//     return (
//         <div id="tabuleiro">
//             {jogo.tabuleiro().flatMap((texto, linha) =>
//                 Array.from(texto).map((peca, coluna) => {
//                     const casa = nomeDaCasa(linha, coluna)
//                     return (
//                         <Casa
//                             key={casa}
//                             row={linha}
//                             col={coluna}
//                             peca={peca}
//                             selecionada={casa === selecionada}
//                             destino={destinos.includes(casa)}
//                             aoClicar={() => aoClicar(casa)}
//                         />
//                     )
//                 })
//             )}
//         </div>
//     )
// }
// export default Tabuleiro
