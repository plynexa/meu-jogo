#!/usr/bin/env bash
set -euo pipefail

arquivo="servidor/src/connection.cpp"
servidor="servidor/src/server.cpp"

# Boost.Asio atual removeu membros antigos de io_context; use as funcoes livres.
sed -i 's/m_service\.dispatch(boost::bind(&Connection::onStop, this));/boost::asio::dispatch(m_service, boost::bind(\&Connection::onStop, this));/' "$arquivo"
sed -i 's/m_io_service\.post(boost::bind(&ServicePort::close, it->second));/boost::asio::post(m_io_service, boost::bind(\&ServicePort::close, it->second));/' "$servidor"

# Boost.DateTime atual nao trata enum anonimo como tipo integral neste construtor.
sed -i 's/boost::posix_time::seconds(Connection::readTimeout)/boost::posix_time::seconds(static_cast<int64_t>(Connection::readTimeout))/g' "$arquivo"
sed -i 's/boost::posix_time::seconds(Connection::writeTimeout)/boost::posix_time::seconds(static_cast<int64_t>(Connection::writeTimeout))/g' "$arquivo"

# address_v4::to_ulong foi removido nas versoes atuais. Restrinja a troca ao endereco IPv4
# para nao tocar em std::bitset::to_ulong(), que continua correto.
find servidor/src -type f \( -name '*.cpp' -o -name '*.h' \) -print0 | xargs -0 sed -i 's/\.to_v4()\.to_ulong()/\.to_v4()\.to_uint()/g'

echo 'Camada de compatibilidade Boost aplicada.'
