// Copyright (c) 2016 Genoil <jw@meneer.net>
// Copyright (c) 2016 Jack Grigg <jack@z.cash>
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "StratumClient.h"
#include "version.h"
#include "streams.h"
//#include "util.h"

#include "utilstrencodings.h"
#include "util.h"
#include "json/json_spirit_reader_template.h"
#include "json/json_spirit_utils.h"

using boost::asio::ip::tcp;
using namespace json_spirit;

template <typename Miner, typename Job, typename Solution>
StratumClient<Miner, Job, Solution>::StratumClient(
		boost::asio::io_service& io_s, Miner * m,
        string const & host, string const & port,
        string const & user, string const & pass,
        int const & retries, int const & worktimeout)
    : m_io_service(io_s)
	, m_socket(m_io_service)
{
    m_primary.host = host;
    m_primary.port = port;
    m_primary.user = user;
    m_primary.pass = pass;

    p_active = &m_primary;

    m_authorized = false;
    m_connected = false;
    m_maxRetries = retries;
    m_worktimeout = worktimeout;

    p_miner = m;

	startWorking();
}

template <typename Miner, typename Job, typename Solution>
void StratumClient<Miner, Job, Solution>::setFailover(
        string const & host, string const & port)
{
    setFailover(host, port, p_active->user, p_active->pass);
}

template <typename Miner, typename Job, typename Solution>
void StratumClient<Miner, Job, Solution>::setFailover(
        string const & host, string const & port,
        string const & user, string const & pass)
{
    m_failover.host = host;
    m_failover.port = port;
    m_failover.user = user;
    m_failover.pass = pass;
}

template <typename Miner, typename Job, typename Solution>
void StratumClient<Miner, Job, Solution>::startWorking()
{
	m_io_work.reset(new boost::asio::io_service::work(m_io_service));

	this->workLoop(boost::system::error_code(), boost::asio::coroutine());
}


template<typename Handler>
struct connect_op : boost::asio::coroutine
{
	connect_op(tcp::socket& socket, std::string host, std::string port, Handler handler)
		: _socket(socket)
		, m_handler(handler)
		, q(host, port)
	{
		r.reset(new tcp::resolver(socket.get_io_context()));
	}


	void operator()(boost::system::error_code ec = boost::system::error_code(), tcp::resolver::iterator endpoint_iterator = tcp::resolver::iterator())
	{
		if (ec)
		{
			LogPrintf("Could not connect to stratum server %s ", ec.message());
			m_handler(ec);
			return;
		}

		BOOST_ASIO_CORO_REENTER(this)
		{
			BOOST_ASIO_CORO_YIELD r->async_resolve(q, *this);

			BOOST_ASIO_CORO_YIELD boost::asio::async_connect(_socket, endpoint_iterator, tcp::resolver::iterator(), *this);


			LogPrintf("Connected!");

			m_handler(ec);
		}
	}

	tcp::socket& _socket;

	std::shared_ptr<tcp::resolver> r;
	tcp::resolver::query q;

	Handler m_handler;
};

template <typename Miner, typename Job, typename Solution>
template <typename Handler>
void StratumClient<Miner, Job, Solution>::async_connect(Handler handler)
{
	LogPrintf("Connecting to stratum server %s:%s", p_active->host, p_active->port);

	connect_op<Handler>(this->m_socket, p_active->host, p_active->port, handler)();
}

template <typename Miner, typename Job, typename Solution>
void StratumClient<Miner, Job, Solution>::workLoop(boost::system::error_code ec, boost::asio::coroutine coro)
{
	if (ec == boost::asio::error::operation_aborted)
	{
		return;
	}

	BOOST_ASIO_CORO_REENTER(coro)
	{
		if (!p_miner->isMining()) {
			LogPrintf("Starting miner");
			p_miner->start();
		}

		BOOST_ASIO_CORO_YIELD this->async_connect(boost::bind(&StratumClient<Miner, Job, Solution>::workLoop, this, _1, coro));

		if (ec)
		{
			std::cerr << ec.message() << std::endl;
			p_current.reset();
			report_error(ec.message());
			m_reconnect_delay = 3000;
			reconnect();
			return;
		}

		{
			m_connected = true;
			m_retries = 0;
			m_socket.set_option(boost::asio::socket_base::keep_alive(true));
			m_socket.set_option(boost::asio::ip::tcp::no_delay(true));
			std::stringstream ss;
			ss << "{\"id\":1,\"method\":\"mining.subscribe\",\"params\":[\""
			<< p_miner->userAgent() << "\", null,\""
			<< p_active->host << "\",\""
			<< p_active->port << "\"]}\n";
			std::string sss = ss.str();
			std::ostream os(&m_requestBuffer);
			os << sss;

			write(m_socket, m_requestBuffer);

			m_share_id = 4;
		}

		while (m_running)
		{
			{
				BOOST_ASIO_CORO_YIELD async_read_until(m_socket, m_responseBuffer, "\n", boost::bind(&StratumClient::workLoop, this, _1, coro));
			}

			if (ec)
			{
				std::cerr << ec.message() << std::endl;
				p_current.reset();
				m_reconnect_delay = 300;
				reconnect();
				return;
			}
			else
			{

				std::istream is(&m_responseBuffer);
				std::string response;
				getline(is, response);

				if (!response.empty() && response.front() == '{' && response.back() == '}')
				{
					Value valResponse;
					std::cerr << response << std::endl;
					if (read_string(response, valResponse) && valResponse.type() == obj_type)
					{
						const Object& responseObject = valResponse.get_obj();
						if (!responseObject.empty())
						{
							processReponse(responseObject);
							m_response = response;
						}
						else
						{
							//LogS("[WARN] Response was empty\n");
						}
					}
					else
					{
						//LogS("[WARN] Parse response failed\n");
					}
				}
				else
				{
					//LogS("[WARN] Discarding incomplete response\n");
				}
			}
		}
	}
}

template <typename Miner, typename Job, typename Solution>
void StratumClient<Miner, Job, Solution>::reconnect()
{
	/*if (p_miner->isMining()) {
		BOOST_LOG_CUSTOM(info) << "Stopping miner";
		p_miner->stop();
	}*/
	p_miner->setJob(nullptr);

    //m_io_service.reset();
    //m_socket.close(); // leads to crashes on Linux
    m_authorized = false;
    m_connected = false;

    if (!m_failover.host.empty()) {
        m_retries++;

        if (m_retries > m_maxRetries) {
            if (m_failover.host == "exit") {
                disconnect();
                return;
            } else if (p_active == &m_primary) {
                p_active = &m_failover;
            } else {
                p_active = &m_primary;
            }
            m_retries = 0;
        }
    }

    LogPrintf("Reconnecting in 3 seconds...");
    boost::asio::deadline_timer timer(m_io_service, boost::posix_time::milliseconds(m_reconnect_delay));
	m_reconnect_delay = 3000;
    timer.wait();
	startWorking();
}

template <typename Miner, typename Job, typename Solution>
void StratumClient<Miner, Job, Solution>::disconnect()
{
    if (!m_connected) return;

    LogPrintf("Disconnecting");
    m_connected = false;
    m_running = false;
    if (p_miner->isMining()) {
        LogPrintf("Stopping miner");
        p_miner->stop();
    }
    m_socket.close();
	m_io_work.reset();
}

template <typename Miner, typename Job, typename Solution>
void StratumClient<Miner, Job, Solution>::processReponse(const Object& responseObject)
{
    const Value& valError = find_value(responseObject, "error");
    if (valError.type() == array_type) {
        const Array& error = valError.get_array();
        string msg;
        if (error.size() > 0 && error[1].type() == str_type) {
            msg = error[1].get_str();
        } else {
            msg = "Unknown error";
        }
        //LogS("%s\n", msg);
    }
    std::ostream os(&m_requestBuffer);
	std::stringstream ss;
    const Value& valId = find_value(responseObject, "id");
    int id = 0;
    if (valId.type() == int_type) {
        id = valId.get_int();
    }
    Value valRes;
    bool accepted = false;
    switch (id) {
	case 0:
	{
		const Value& valMethod = find_value(responseObject, "method");
		string method = "";
		if (valMethod.type() == str_type) {
			method = valMethod.get_str();
		}

		if (method == "mining.notify") {
			const Value& valParams = find_value(responseObject, "params");
			if (valParams.type() == array_type) {
				const Array& params = valParams.get_array();
				std::shared_ptr<Job> workOrder = p_miner->parseJob(params);

				if (workOrder)
				{
					if (!workOrder->clean)
					{
						if (p_current)
						{
							// check for time, if time diff to much, mine for new block.
							if ((p_current->header.nTime + 123) > workOrder->header.nTime)
								break;
						}
					}

					workOrder->setTarget(m_nextJobTarget);

					if (p_current && *workOrder == *p_current){
						break;
					}

					p_previous = p_current;
					p_current = std::move(workOrder);

					p_miner->setJob(p_current);
				}
			}
		}
		else if (method == "mining.set_target") {
			const Value& valParams = find_value(responseObject, "params");
			if (valParams.type() == array_type) {
				const Array& params = valParams.get_array();
				m_nextJobTarget = params[0].get_str();
				new_target(m_nextJobTarget);
			}
		}
		else if (method == "mining.set_extranonce") {
			const Value& valParams = find_value(responseObject, "params");
			if (valParams.type() == array_type) {
				const Array& params = valParams.get_array();
				p_miner->setServerNonce(params[0].get_str());
			}
		}
		else if (method == "client.reconnect") {
			const Value& valParams = find_value(responseObject, "params");
			if (valParams.type() == array_type) {
				const Array& params = valParams.get_array();
				if (params.size() > 1) {
					p_active->host = params[0].get_str();
					p_active->port = params[1].get_str();
				}
				// TODO: Handle wait time
		 		p_current.reset();
				//reconnect();
			}
		}
		break;
	}
    case 1:
        valRes = find_value(responseObject, "result");
        if (valRes.type() == array_type) {
			const Array& result = valRes.get_array();
            // Ignore session ID for now.
            p_miner->setServerNonce(result[1].get_str());
			ss << "{\"id\":2,\"method\":\"mining.authorize\",\"params\":[\""
			   << p_active->user << "\",\"" << p_active->pass << "\"]}\n";
			std::string sss = ss.str();
			os << sss;
            write(m_socket, m_requestBuffer);

			const Array& command_list = result[0].get_array();

			if (command_list[0].get_array()[0].get_str() == "mining.set_target")
			{
				m_nextJobTarget = command_list[0].get_array()[1].get_str();
				new_target(m_nextJobTarget);
			}

        }
        break;
	case 2:
	{
		valRes = find_value(responseObject, "result");
		m_authorized = false;
		if (valRes.type() == bool_type) {
			m_authorized = valRes.get_bool();
		}
		if (!m_authorized) {

			auto valRes = find_value(responseObject, "error");

			if (valRes.type() == array_type)
			{
				const Array& params = valRes.get_array();
				if (params.size() > 1 && params[1].type() == str_type)
				{
					auto reason = params[1].get_str();
					report_error(reason);
				}
			}

			disconnect();
			return;
		}

	//	ss << "{\"id\":3,\"method\":\"mining.extranonce.subscribe\",\"params\":[]}\n";
	//	std::string sss = ss.str();
	//	os << sss;
	//	write(m_socket, m_requestBuffer);

		break;
	}
    case 3:
        // nothing to do...
        break;
    default:
        valRes = find_value(responseObject, "result");
        if (valRes.type() == bool_type) {
            accepted = valRes.get_bool();
        }
        if (accepted) {
            p_miner->acceptedSolution(m_stale);
        } else {
			valRes = find_value(responseObject, "error");
			std::string reason = "unknown";
			if (valRes.type() == array_type)
			{
				const Array& params = valRes.get_array();
				if (params.size() > 1 && params[1].type() == str_type)
					reason = params[1].get_str();
			}
            p_miner->rejectedSolution(m_stale);
			if (reason != "Job not found (=stale)")
			{
				m_reconnect_delay = 30;
				reconnect();
			}
        }
        break;

    }
}

template <typename Miner, typename Job, typename Solution>
void StratumClient<Miner, Job, Solution>::work_timeout_handler(
        const boost::system::error_code& ec)
{
    if (!ec) {
        //LogS("No new work received in %d seconds.\n", m_worktimeout);
        reconnect();
    }
}

template <typename Miner, typename Job, typename Solution>
bool StratumClient<Miner, Job, Solution>::submit(const Solution* solution, const std::string& jobid)
{
	int id = std::atomic_fetch_add(&m_share_id, 1);

	std::stringstream stream;
	stream << R"json({"id":")json" << id << R"json(", "method":"mining.submit","params":[")json";
	stream << p_active->user;
	stream << "\",\"" << jobid;
	stream << "\",\"" << solution->time;
	stream << "\",\"" << HexStr(solution->nonce);
	stream << "\",\"" << HexStr(solution->solution);
	stream << "\"]}\n";
	std::string json = stream.str();
	std::ostream os(&m_requestBuffer);
	os << json;
	write(m_socket, m_requestBuffer);
	return true;
}

// create StratumClient class
template class StratumClient<ZcashMiner, ZcashJob, EquihashSolution>;
