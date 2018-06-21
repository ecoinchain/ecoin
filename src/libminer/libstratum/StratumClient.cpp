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
StratumClient<Miner, Job, Solution>::StratumClient(boost::asio::io_service& io_s, Miner * m,
	std::vector<string> locations, string const & user, string const & pass, int const & retries, int const & worktimeout)
	: m_io_service(io_s)
	, m_socket(m_io_service)
{
	for (auto l : locations)
	{
		cred_t c;
		c.location = l;
		c.user = user;
		c.pass = pass;
		m_servers.push_back(c);
	}

	m_authorized = false;
	m_maxRetries = retries;
	m_worktimeout = worktimeout;

	p_miner = m;

	startWorking();
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
	connect_op(tcp::socket& socket, std::vector<cred_t> locations, int& cur_location_idx, Handler handler)
		: _socket(socket)
		, m_handler(handler)
		, m_locations(locations)
		, cur_location_idx(cur_location_idx)
	{
		r.reset(new tcp::resolver(socket.get_io_service()));
		m_cancal_connect_timer = std::make_shared<boost::asio::steady_timer>(socket.get_io_context());
	}


	void operator()(boost::system::error_code ec = boost::system::error_code(), tcp::resolver::iterator endpoint_iterator = tcp::resolver::iterator())
	{
		BOOST_ASIO_CORO_REENTER(this)
		{
			while(true)
			{
				// pick a locations
				q = from_string(m_locations[cur_location_idx].location);

				BOOST_ASIO_CORO_YIELD r->async_resolve(*q, *this);

				if (ec)
				{
					if (m_locations.size()>1)
					{
						cur_location_idx++;
						m_tried ++;
						cur_location_idx %= m_locations.size();

						if (m_tried == m_locations.size())
						{
							LogPrintf("Could not connect to stratum server: %s\n", ec.message());
							m_handler(ec);
							return;
						}

						LogPrintf("Could not connect to stratum server: %s, try next server %s\n", ec.message(), m_locations[cur_location_idx].location);
						continue;
					}

					LogPrintf("Could not connect to stratum server: %s\n", ec.message());
					m_handler(ec);
					return;
				}

				LogPrintf("Connecting to stratum server %s\n", m_locations[cur_location_idx].location);

				m_cancal_connect_timer->expires_from_now(std::chrono::seconds(5));
				{
					tcp::socket& ts = _socket;
					m_cancal_connect_timer->async_wait([&ts](boost::system::error_code ec)
					{
						if (!ec )
						{
							boost::system::error_code ignored_ec;
							ts.close(ignored_ec);
						}
					});
				}
				BOOST_ASIO_CORO_YIELD boost::asio::async_connect(_socket, endpoint_iterator, tcp::resolver::iterator(), *this);

				if (ec)
				{
					if (ec == boost::asio::error::operation_aborted)
					{
						ec = boost::asio::error::make_error_code(boost::asio::error::timed_out);

						if (m_locations.size()>1)
						{
							cur_location_idx++;
							m_tried ++;
							cur_location_idx %= m_locations.size();

							if (m_tried == m_locations.size())
							{
								m_handler(ec);
								return;
							}

							LogPrintf("Could not connect to stratum server: %s, try next server %s\n", ec.message(), m_locations[cur_location_idx].location);
							continue;
						}
					}
					else
					{
						boost::system::error_code ignore_ec;
						m_cancal_connect_timer->cancel(ignore_ec);
					}
					LogPrintf("Could not connect to stratum server: %s\n", ec.message());
					m_handler(ec);
					return;
				}

				m_cancal_connect_timer->cancel(ec);

				LogPrintf("Connected!\n");

				m_handler(ec);
				return;
			}
		}
	}

	static std::shared_ptr<tcp::resolver::query> from_string(std::string location)
	{
		size_t delim = location.find(':');
		std::string host = delim != std::string::npos ? location.substr(0, delim) : location;
		std::string port = delim != std::string::npos ? location.substr(delim + 1) : "3333";

		return std::make_shared<tcp::resolver::query>(host, port);
	}

	tcp::socket& _socket;

	std::shared_ptr<tcp::resolver> r;
	std::shared_ptr<tcp::resolver::query> q;

	Handler m_handler;

	int& cur_location_idx;
	int m_tried = 0;
	std::vector<cred_t> m_locations;
	std::shared_ptr<boost::asio::steady_timer> m_cancal_connect_timer;
};

template <typename Miner, typename Job, typename Solution>
template <typename Handler>
void StratumClient<Miner, Job, Solution>::async_connect(Handler handler)
{
	connect_op<Handler>(this->m_socket, m_servers, server_active_idx, handler)();
}


#include <boost/locale.hpp>
#include <boost/locale/utf.hpp>

#ifndef _WIN32
inline std::wstring ansi_wide(const std::string& source)
{
	std::wstring wide;
	wchar_t dest;
	std::size_t max = source.size();

	// reset mbtowc.
	mbtowc(NULL, NULL, 0);

	// convert source to wide.
	for (std::string::const_iterator i = source.begin();
		i != source.end(); )
	{
		int length = mbtowc(&dest, &(*i), max);
		if (length < 1)
			break;
		max -= length;
		while (length--) i++;
		wide.push_back(dest);
	}

	return wide;
}
#else
inline std::wstring ansi_wide(const std::string& source)
{
	std::wstring wide;

    int len = ::MultiByteToWideChar(CP_ACP, 0, source.c_str(), source.length(), NULL, 0);
    if (len == 0) return L"";

    wide.resize(len);
    ::MultiByteToWideChar(CP_ACP, 0, source.c_str(), source.length(), &wide[0], len);

    return wide;
}
#endif

inline std::string wide_utf8(const std::wstring& source)
{
	return boost::locale::conv::utf_to_utf<char>(source);
}

inline std::string ansi_utf8(std::string const& source)
{
	std::wstring wide = ansi_wide(source);
	return wide_utf8(wide);
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
			LogPrintf("Starting miner\n");
			p_miner->start();
		}

		BOOST_ASIO_CORO_YIELD this->async_connect(boost::bind(&StratumClient<Miner, Job, Solution>::workLoop, this, _1, coro));

		if (ec)
		{
			std::cerr << ec.message() << std::endl;
			p_current.reset();
#ifdef _WIN32
			report_error(ansi_utf8(ec.message()), false);
#else
			report_error(ec.message(), false);
#endif
			m_reconnect_delay = 3000;
			reconnect();
			return;
		}

		{
			m_retries = 0;
			m_socket.set_option(boost::asio::socket_base::keep_alive(true));
			m_socket.set_option(boost::asio::ip::tcp::no_delay(true));
			std::stringstream ss;
			ss << "{\"id\":1,\"method\":\"mining.subscribe\",\"params\":[\""
			<< p_miner->userAgent() << "\", null,\"\", \"\"]}\n";
			std::string sss = ss.str();
			std::ostream os(&m_requestBuffer);
			os << sss;

			boost::system::error_code ec;
			boost::asio::write(m_socket, m_requestBuffer, ec);

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
							m_response = response;
							processReponse(responseObject);
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

	server_active_idx++;
	server_active_idx %= m_servers.size();

	m_retries++;

	LogPrintf("Reconnecting in 3 seconds...\n");
	boost::asio::deadline_timer timer(m_io_service, boost::posix_time::milliseconds(m_reconnect_delay));
	m_reconnect_delay = 3000;
	timer.wait();
	startWorking();
}

template <typename Miner, typename Job, typename Solution>
void StratumClient<Miner, Job, Solution>::disconnect()
{
	boost::system::error_code ignore;
    LogPrintf("Disconnecting\n");
    m_running = false;
    if (p_miner->isMining()) {
        LogPrintf("Stopping miner\n");
        p_miner->stop();
    }
    m_socket.close(ignore);
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

			dismiss_error();

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
			}
		}
		else if (method == "mining.set_extranonce") {
			const Value& valParams = find_value(responseObject, "params");
			if (valParams.type() == array_type) {
				const Array& params = valParams.get_array();
				p_miner->setServerNonce(params[0].get_str());
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
			   << m_servers[server_active_idx].user << "\",\"" << m_servers[server_active_idx].pass << "\"]}\n";
			std::string sss = ss.str();
			os << sss;
			boost::system::error_code ec;
            boost::asio::write(m_socket, m_requestBuffer, ec);

			const Array& command_list = result[0].get_array();

			if (command_list[0].get_array()[0].get_str() == "mining.set_target")
			{
				m_nextJobTarget = command_list[0].get_array()[1].get_str();
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
					report_error(reason, false);
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
	stream << m_servers[server_active_idx].user;
	stream << "\",\"" << jobid;
	stream << "\",\"" << solution->time;
	stream << "\",\"" << HexStr(solution->nonce);
	stream << "\",\"" << HexStr(solution->solution);
	stream << "\"]}\n";
	std::string json = stream.str();
	std::ostream os(&m_requestBuffer);
	os << json;
	boost::system::error_code ec;
	boost::asio::write(m_socket, m_requestBuffer, ec);
	return true;
}

// create StratumClient class
template class StratumClient<ZcashMiner, ZcashJob, EquihashSolution>;
