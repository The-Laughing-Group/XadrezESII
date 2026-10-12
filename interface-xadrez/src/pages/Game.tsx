import Tabuleiro from "../components/Tabuleiro"
import { usePartida } from "../hooks/usePartida"

const Game = () => {
    const partida = usePartida();
    // console.log(partida.fen);      // muda a cada lance aceito

    // passar partida como parametro para sempre ser atualizada e utilizada a mesma referencia do tabuleiro
    return (
        <div className="game">
            <Tabuleiro partida={partida} />
        </div>
    )
}
export default Game