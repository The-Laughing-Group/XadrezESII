import { Outlet } from "react-router-dom"
import FundoPecas from "./FundoPecas"

const LayoutFundo = () => {
  return (
    <div id="layout-fundo">
        <FundoPecas />
        <Outlet />
    </div>
  )
}
export default LayoutFundo