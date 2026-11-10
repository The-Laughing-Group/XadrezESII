import { Link, useNavigate } from "react-router-dom";
import { useState } from "react";

import voltar from "../assets/icones/voltar.png"
import twoplayers from "../assets/icones/2players.png";
import ia from "../assets/icones/ia.png";

type Modo = "twoplayers" | "ia";
type Dificuldade = "facil" | "medio" | "dificil";

const DIFICULDADES: { valor: Dificuldade; rotulo: string }[] = [
  { valor: "facil", rotulo: "Fácil" },
  { valor: "medio", rotulo: "Médio" },
  { valor: "dificil", rotulo: "Difícil" },
];

const Modos = () => {
  const navigate = useNavigate();

  const [modo, setModo] = useState<Modo>("twoplayers");
  const [dificuldade, setDificuldade] = useState<Dificuldade>("medio");

  const usaIA = modo === "ia";

  function jogar() {
    const params = new URLSearchParams({ modo });
    if (usaIA) params.set("dificuldade", dificuldade);
    navigate(`/game?${params.toString()}`);
  }

  return (
    <div id="modos">
      <Link className="botao-pixel" to="/" aria-label="Voltar" title="Voltar">
        <img src={voltar} className="icone-botao" alt="" draggable={false} />
      </Link>

      <div className="conteudo-modos">
        <div className="opcoes-modo">
          <button
            type="button"
            role="checkbox"
            aria-checked={modo === "twoplayers"}
            className={`botao-pixel-modo ${modo === "twoplayers" ? "selecionado" : ""}`}
            onClick={() => setModo("twoplayers")}
          >
            <span className="marca" aria-hidden="true" />
            <img src={twoplayers} alt="" draggable={false} />
            <p>2 Players</p>
          </button>

          <button
            type="button"
            role="checkbox"
            aria-checked={modo === "ia"}
            className={`botao-pixel-modo ${modo === "ia" ? "selecionado" : ""}`}
            onClick={() => setModo("ia")}
          >
            <span className="marca" aria-hidden="true" />
            <img src={ia} alt="" draggable={false} />
            <p>Player x IA</p>
          </button>
        </div>

        <div className={`grupo-config grupo-dificuldade ${modo === "ia" ? "selecionado" : ""}`}>
          <h2>Dificuldade</h2>
          <div>
            {DIFICULDADES.map(({ valor, rotulo }) => (
              <label
                key={valor}
                className={`linha-config ${!usaIA ? "desabilitado" : ""}`}
              >
                <span>{rotulo}</span>
                <input
                  type="checkbox"
                  className="interruptor"
                  disabled={!usaIA}
                  checked={usaIA && dificuldade === valor}
                  onChange={() => setDificuldade(valor)}
                />
              </label>
            ))}
          </div>
        </div>

        <button type="button" className="botao-pixel" onClick={jogar}>
          Jogar
        </button>
      </div>
    </div>
  );
};
export default Modos;
