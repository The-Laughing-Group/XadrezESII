import { Link } from "react-router-dom";

import twoplayers from "../assets/icones/2players.png";
import ia from "../assets/icones/ia.png";
import FundoPecas from "../components/FundoPecas";

const Modos = () => {
  return (
    <div id="modos">
      <FundoPecas />
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
