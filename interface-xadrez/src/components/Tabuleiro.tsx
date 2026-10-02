import Casa from "./Casa.tsx"

const Tabuleiro = () => {
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
