import { useLocation } from 'react-router-dom'

export default function LocalAtual() {
  const { pathname, search } = useLocation()
  return <div data-testid="local">{pathname + search}</div>
}