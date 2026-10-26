import { Link } from "react-router-dom";

import voltar from "../assets/icones/voltar.png"
import twoplayers from "../assets/icones/2players.png";
import ia from "../assets/icones/ia.png";

const Modos = () => {
  return (
    <div id="modos">
      <Link className="botao-pixel" to="/"
          aria-label="Voltar" title="Voltar">
          <img src={voltar} className="icone-botao" alt="" draggable={false} />
      </Link>

      <Link to="/game?modo=twoplayers" className="botao-pixel-modo">
        <img src={twoplayers} alt="" />
        <p>2 Players</p>
      </Link>

      <Link to="/game?modo=ia" className="botao-pixel-modo">
        <img src={ia} alt="" />
        <p>Player x IA</p>
      </Link>
    </div>
  );
};
export default Modos;
