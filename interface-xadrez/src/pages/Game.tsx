import { useState } from "react";
import Tabuleiro from "../components/Tabuleiro"
import { usePartida } from "../hooks/usePartida"

const Game = () => {
    const partida = usePartida();

    const [historicoMovimentos] = useState<(string)[]>([]);
    
    return (
        <div className="game">
            <Tabuleiro partida={partida} historicoMovimentos={historicoMovimentos} />
            <div className={`indicador-vez ${partida.vezDasBrancas ? "indicador-vez-branca" : "indicador-vez-preta"}`}>Vez das {partida.vezDasBrancas ? "brancas" : "pretas"}</div>
            <div id="historico-movimentos">
                <div className="coluna-movimentos">
                    <h3>Rodadas</h3>
                    {historicoMovimentos
                        .filter((_, index) => index % 2 === 0)
                        .map((_,index) => (
                            <p key={index + 1}>
                                {index + 1}. 
                            </p>
                        ))}
                </div>
                <div className="coluna-movimentos">
                    <h3>Brancas</h3>

                    {historicoMovimentos
                        .filter((_, index) => index % 2 === 0)
                        .map((movimento, index) => (
                            <p key={index}>
                                {movimento}
                            </p>
                        ))}
                </div>
                <div className="coluna-movimentos">
                    <h3>Pretas</h3>
                    {historicoMovimentos
                        .filter((_, index) => index % 2 !== 0)
                        .map((movimento, index) => (
                            <p key={index + 1}>
                                {movimento}
                            </p>
                        ))}
                </div>
            </div>
        </div>
    )
}
export default Game