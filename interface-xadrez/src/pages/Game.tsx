import { useEffect, useState } from "react";
import { useSearchParams } from "react-router-dom";

import Tabuleiro from "../components/Tabuleiro"

import { usePartida } from "../hooks/usePartida"
import { useMusica } from "../hooks/useMusica";

const Game = () => {
    useMusica();
    
    const partida = usePartida();
    const [searchParams] = useSearchParams();
    const modoIA = searchParams.get("modo") === "ia";
    
    let profundidade : number;
    if(modoIA) {
        switch(searchParams.get("dificuldade")) {
            case "facil": { profundidade = 2; break; }
            case "medio": { profundidade = 3; break; }
            case "dificil": { profundidade = 5; break; }
        }
    }

    const [historicoMovimentos] = useState<string[]>([]);

    useEffect(() => {
      if (
        !modoIA ||
        !partida.pronta ||
        partida.vezDasBrancas ||
        partida.situacao.terminou ||
        !partida.fen
      )
        return;

      partida.jogarIA(profundidade);
    }, [
      modoIA,
      partida.pronta,
      partida.vezDasBrancas,
      partida.situacao.terminou,
      partida.fen,
      partida.jogarIA,
    ]);

    return (
      <div id="game">
        <div className="painel">
          <Tabuleiro
            partida={partida}
            historicoMovimentos={historicoMovimentos}
            contraIA={modoIA}
          />
        </div>
        <div id="historico-movimentos">
          <div className="coluna-movimentos">
            <h3>Rodadas</h3>
            {historicoMovimentos
              .filter((_, index) => index % 2 === 0)
              .map((_, index) => (
                <p key={index + 1}>{index + 1}.</p>
              ))}
          </div>
          <div className="coluna-movimentos">
            <h3>Brancas</h3>

            {historicoMovimentos
              .filter((_, index) => index % 2 === 0)
              .map((movimento, index) => (
                <p key={index}>{movimento}</p>
              ))}
          </div>
          <div className="coluna-movimentos">
            <h3>Pretas</h3>
            {historicoMovimentos
              .filter((_, index) => index % 2 !== 0)
              .map((movimento, index) => (
                <p key={index + 1}>{movimento}</p>
              ))}
          </div>
        </div>
      </div>
    );
};
export default Game;
