import { Link } from "react-router-dom"
import logo from "../assets/logo.gif"
import config from "../assets/icones/config.png"
import K from "../assets/pecas/reiBranco.png"

const Inicial = () => {
    return (
        <div id="inicial">
            <img src={logo} className="logo-jogo" alt="Nome do jogo" draggable={false}/>
            <nav className="menu-inicial">
                <Link className="botao-pixel" to="/modos"><img className="icone-botao icone-inclinado" src={K}/>Jogar</Link>
                <Link className="botao-pixel" to="/configuracoes"><img className="icone-botao" src={config}/>Configurações</Link>
            </nav>
        </div>
    )
}
export default Inicial;