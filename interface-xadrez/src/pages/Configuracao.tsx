import { Link } from "react-router-dom";      
import voltar from "../assets/icones/voltar.png"
import PainelConfiguracoes from "../components/PainelConfiguracoes";

const Configuracao = () => {
    return (
        <div id="configuracoes">
            <Link className="botao-pixel" to="/"
                aria-label="Voltar" title="Voltar">
                <img src={voltar} className="icone-botao" alt="" draggable={false} />
            </Link>

            <h1>Configurações</h1>

            <PainelConfiguracoes />
        </div>
    )
}
export default Configuracao;
