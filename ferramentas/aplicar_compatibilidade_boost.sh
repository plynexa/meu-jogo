#!/usr/bin/env bash
set -euo pipefail

arquivo="servidor/src/connection.cpp"

# Boost.Asio atual removeu membros antigos de io_context; use a funcao livre.
sed -i 's/m_service\.dispatch(boost::bind(&Connection::onStop, this));/boost::asio::dispatch(m_service, boost::bind(\&Connection::onStop, this));/' "$arquivo"

# Boost.DateTime atual nao trata enum anonimo como tipo integral neste construtor.
sed -i 's/boost::posix_time::seconds(Connection::readTimeout)/boost::posix_time::seconds(static_cast<int64_t>(Connection::readTimeout))/g' "$arquivo"
sed -i 's/boost::posix_time::seconds(Connection::writeTimeout)/boost::posix_time::seconds(static_cast<int64_t>(Connection::writeTimeout))/g' "$arquivo"

# address_v4::to_ulong foi removido nas versoes atuais; to_uint preserva a semantica desejada.
find servidor/src -type f \( -name '*.cpp' -o -name '*.h' \) -print0 | xargs -0 sed -i 's/\.to_ulong()/\.to_uint()/g'

echo 'Camada de compatibilidade Boost aplicada.'
