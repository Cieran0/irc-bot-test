#pragma once
#include <unordered_map>
#include <string>
//Generated from https://www.alien.net.au/irc/irc2numerics.html
namespace irc {
	enum numeric_reply {
		/*The first message sent after client registration. The text used varies widely*/
		RPL_WELCOME = 1, //Format: :Welcome to the Internet Relay Network <nick>!<user>@<host>
		/*Part of the post-registration greeting. Text varies widely*/
		RPL_YOURHOST = 2, //Format: :Your host is <servername>, running version <version>
		/*Part of the post-registration greeting. Text varies widely*/
		RPL_CREATED = 3, //Format: :This server was created <date>
		/*Part of the post-registration greeting*/
		RPL_MYINFO = 4, //Format: <server_name> <version> <user_modes> <chan_modes>
		/*Sent by the server to a user to suggest an alternative server, sometimes used when the connection is refused because the server is already full. Also known as RPL_SLINE (AustHex), and RPL_REDIR Also see #010.*/
		RPL_BOUNCE = 5, //Format: :Try server <server_name>, port <port_number>
		/*See RFC*/
		RPL_TRACELINK = 200, //Format: Link <version>[.<debug_level>] <destination> <next_server> [V<protocol_version> <link_uptime_in_seconds> <backstream_sendq> <upstream_sendq>]
		/*See RFC*/
		RPL_TRACECONNECTING = 201, //Format: Try. <class> <server>
		/*See RFC*/
		RPL_TRACEHANDSHAKE = 202, //Format: H.S. <class> <server>
		/*See RFC*/
		RPL_TRACEUNKNOWN = 203, //Format: ???? <class> [<connection_address>]
		/*See RFC*/
		RPL_TRACEOPERATOR = 204, //Format: Oper <class> <nick>
		/*See RFC*/
		RPL_TRACEUSER = 205, //Format: User <class> <nick>
		/*See RFC*/
		RPL_TRACESERVER = 206, //Format: Serv <class> <int>S <int>C <server> <nick!user|*!*>@<host|server> [V<protocol_version>]
		/*See RFC*/
		RPL_TRACESERVICE = 207, //Format: Service <class> <name> <type> <active_type>
		/*See RFC*/
		RPL_TRACENEWTYPE = 208, //Format: <newtype> 0 <client_name>
		/*See RFC*/
		RPL_TRACECLASS = 209, //Format: Class <class> <count>
		/**/
		RPL_TRACERECONNECT = 210, //Format: 
		/*Reply to STATS (See RFC)*/
		RPL_STATSLINKINFO = 211, //Format: <linkname> <sendq> <sent_msgs> <sent_bytes> <recvd_msgs> <rcvd_bytes> <time_open>
		/*Reply to STATS (See RFC)*/
		RPL_STATSCOMMANDS = 212, //Format: <command> <count> [<byte_count> <remote_count>]
		/*Reply to STATS (See RFC)*/
		RPL_STATSCLINE = 213, //Format: C <host> * <name> <port> <class>
		/*Reply to STATS (See RFC), Also known as RPL_STATSOLDNLINE (ircu, Unreal)*/
		RPL_STATSNLINE = 214, //Format: N <host> * <name> <port> <class>
		/*Reply to STATS (See RFC)*/
		RPL_STATSILINE = 215, //Format: I <host> * <host> <port> <class>
		/*Reply to STATS (See RFC)*/
		RPL_STATSKLINE = 216, //Format: K <host> * <username> <port> <class>
		/**/
		RPL_STATSQLINE = 217, //Format: 
		/*Reply to STATS (See RFC)*/
		RPL_STATSYLINE = 218, //Format: Y <class> <ping_freq> <connect_freq> <max_sendq>
		/*End of RPL_STATS* list.*/
		RPL_ENDOFSTATS = 219, //Format: <query> :<info>
		/*Information about a user's own modes. Some daemons have extended the mode command and certain modes take parameters (like channel modes).*/
		RPL_UMODEIS = 221, //Format: <user_modes> [<user_mode_params>]
		/**/
		RPL_SERVICEINFO = 231, //Format: 
		/**/
		RPL_ENDOFSERVICES = 232, //Format: 
		/**/
		RPL_SERVICE = 233, //Format: 
		/*A service entry in the service list*/
		RPL_SERVLIST = 234, //Format: <name> <server> <mask> <type> <hopcount> <info>
		/*Termination of an RPL_SERVLIST list*/
		RPL_SERVLISTEND = 235, //Format: <mask> <type> :<info>
		/**/
		RPL_STATSVLINE = 240, //Format: 
		/*Reply to STATS (See RFC)*/
		RPL_STATSLLINE = 241, //Format: L <hostmask> * <servername> <maxdepth>
		/*Reply to STATS (See RFC)*/
		RPL_STATSUPTIME = 242, //Format: :Server Up <days> days <hours>:<minutes>:<seconds>
		/*Reply to STATS (See RFC); The info field is an extension found in some IRC daemons, which returns info such as an e-mail address or the name/job of an operator*/
		RPL_STATSOLINE = 243, //Format: O <hostmask> * <nick> [:<info>]
		/*Reply to STATS (See RFC)*/
		RPL_STATSHLINE = 244, //Format: H <hostmask> * <servername>
		/**/
		RPL_STATSPING = 246, //Format: 
		/**/
		RPL_STATSBLINE = 247, //Format: 
		/**/
		RPL_STATSDLINE = 250, //Format: 
		/*Reply to LUSERS command, other versions exist (eg. RFC2812); Text may vary.*/
		RPL_LUSERCLIENT = 251, //Format: :There are <int> users and <int> invisible on <int> servers
		/*Reply to LUSERS command - Number of IRC operators online*/
		RPL_LUSEROP = 252, //Format: <int> :<info>
		/*Reply to LUSERS command - Number of unknown/unregistered connections*/
		RPL_LUSERUNKNOWN = 253, //Format: <int> :<info>
		/*Reply to LUSERS command - Number of channels formed*/
		RPL_LUSERCHANNELS = 254, //Format: <int> :<info>
		/*Reply to LUSERS command - Information about local connections; Text may vary.*/
		RPL_LUSERME = 255, //Format: :I have <int> clients and <int> servers
		/*Start of an RPL_ADMIN* reply. In practise, the server parameter is often never given, and instead the info field contains the text 'Administrative info about <server>'. Newer daemons seem to follow the RFC and output the server's hostname in the 'server' parameter, but also output the server name in the text as per traditional daemons.*/
		RPL_ADMINME = 256, //Format: <server> :<info>
		/*Reply to ADMIN command (Location, first line)*/
		RPL_ADMINLOC1 = 257, //Format: :<admin_location>
		/*Reply to ADMIN command (Location, second line)*/
		RPL_ADMINLOC2 = 258, //Format: :<admin_location>
		/*Reply to ADMIN command (E-mail address of administrator)*/
		RPL_ADMINEMAIL = 259, //Format: :<email_address>
		/*See RFC*/
		RPL_TRACELOG = 261, //Format: File <logfile> <debug_level>
		/*Used to terminate a list of RPL_TRACE* replies*/
		RPL_TRACEEND = 262, //Format: <server_name> <version>[.<debug_level>] :<info>
		/*When a server drops a command without processing it, it MUST use this reply. Also known as RPL_LOAD_THROTTLED and RPL_LOAD2HI, I'm presuming they do the same thing.*/
		RPL_TRYAGAIN = 263, //Format: <command> :<info>
		/*Dummy reply, supposedly only used for debugging/testing new features, however has appeared in production daemons.*/
		RPL_NONE = 300, //Format: 
		/*Used in reply to a command directed at a user who is marked as away*/
		RPL_AWAY = 301, //Format: <nick> :<message>
		/*Reply used by USERHOST (see RFC)*/
		RPL_USERHOST = 302, //Format: :*1<reply> *( ' ' <reply> )
		/*Reply to the ISON command (see RFC)*/
		RPL_ISON = 303, //Format: :*1<nick> *( ' ' <nick> )
		/*Reply from AWAY when no longer marked as away*/
		RPL_UNAWAY = 305, //Format: :<info>
		/*Reply from AWAY when marked away*/
		RPL_NOWAWAY = 306, //Format: :<info>
		/*Reply to WHOIS - Information about the user*/
		RPL_WHOISUSER = 311, //Format: <nick> <user> <host> * :<real_name>
		/*Reply to WHOIS - What server they're on*/
		RPL_WHOISSERVER = 312, //Format: <nick> <server> :<server_info>
		/*Reply to WHOIS - User has IRC Operator privileges*/
		RPL_WHOISOPERATOR = 313, //Format: <nick> :<privileges>
		/*Reply to WHOWAS - Information about the user*/
		RPL_WHOWASUSER = 314, //Format: <nick> <user> <host> * :<real_name>
		/*Used to terminate a list of RPL_WHOREPLY replies*/
		RPL_ENDOFWHO = 315, //Format: <name> :<info>
		/**/
		RPL_WHOISCHANOP = 316, //Format: 
		/*Reply to WHOIS - Idle information*/
		RPL_WHOISIDLE = 317, //Format: <nick> <seconds> :seconds idle
		/*Reply to WHOIS - End of list*/
		RPL_ENDOFWHOIS = 318, //Format: <nick> :<info>
		/*Reply to WHOIS - Channel list for user (See RFC)*/
		RPL_WHOISCHANNELS = 319, //Format: <nick> :*( ( '@' / '+' ) <channel> ' ' )
		/*Channel list - Header*/
		RPL_LISTSTART = 321, //Format: Channels :Users  Name
		/*Channel list - A channel*/
		RPL_LIST = 322, //Format: <channel> <#_visible> :<topic>
		/*Channel list - End of list*/
		RPL_LISTEND = 323, //Format: :<info>
		/**/
		RPL_CHANNELMODEIS = 324, //Format: <channel> <mode> <mode_params>
		/**/
		RPL_UNIQOPIS = 325, //Format: <channel> <nickname>
		/*Response to TOPIC when no topic is set*/
		RPL_NOTOPIC = 331, //Format: <channel> :<info>
		/*Response to TOPIC with the set topic*/
		RPL_TOPIC = 332, //Format: <channel> :<topic>
		/*Returned by the server to indicate that the attempted INVITE message was successful and is being passed onto the end client. Note that RFC1459 documents the parameters in the reverse order. The format given here is the format used on production servers, and should be considered the standard reply above that given by RFC1459.*/
		RPL_INVITING = 341, //Format: <nick> <channel>
		/*Returned by a server answering a SUMMON message to indicate that it is summoning that user*/
		RPL_SUMMONING = 342, //Format: <user> :<info>
		/*An invite mask for the invite mask list*/
		RPL_INVITELIST = 346, //Format: <channel> <invitemask>
		/*Termination of an RPL_INVITELIST list*/
		RPL_ENDOFINVITELIST = 347, //Format: <channel> :<info>
		/*An exception mask for the exception mask list. Also known as RPL_EXLIST (Unreal, Ultimate)*/
		RPL_EXCEPTLIST = 348, //Format: <channel> <exceptionmask>
		/*Termination of an RPL_EXCEPTLIST list. Also known as RPL_ENDOFEXLIST (Unreal, Ultimate)*/
		RPL_ENDOFEXCEPTLIST = 349, //Format: <channel> :<info>
		/*Reply by the server showing its version details, however this format is not often adhered to*/
		RPL_VERSION = 351, //Format: <version>[.<debuglevel>] <server> :<comments>
		/*Reply to vanilla WHO (See RFC). This format can be very different if the 'WHOX' version of the command is used (see ircu).*/
		RPL_WHOREPLY = 352, //Format: <channel> <user> <host> <server> <nick> <H|G>[*][@|+] :<hopcount> <real_name>
		/*Reply to NAMES (See RFC)*/
		RPL_NAMREPLY = 353, //Format: ( '=' / '*' / '@' ) <channel> ' ' : [ '@' / '+' ] <nick> *( ' ' [ '@' / '+' ] <nick> )
		/**/
		RPL_KILLDONE = 361, //Format: 
		/**/
		RPL_CLOSING = 362, //Format: 
		/**/
		RPL_CLOSEEND = 363, //Format: 
		/*Reply to the LINKS command*/
		RPL_LINKS = 364, //Format: <mask> <server> :<hopcount> <server_info>
		/*Termination of an RPL_LINKS list*/
		RPL_ENDOFLINKS = 365, //Format: <mask> :<info>
		/*Termination of an RPL_NAMREPLY list*/
		RPL_ENDOFNAMES = 366, //Format: <channel> :<info>
		/*A ban-list item (See RFC); <time left> and <reason> are additions used by KineIRCd*/
		RPL_BANLIST = 367, //Format: <channel> <banid> [<time_left> :<reason>]
		/*Termination of an RPL_BANLIST list*/
		RPL_ENDOFBANLIST = 368, //Format: <channel> :<info>
		/*Reply to WHOWAS - End of list*/
		RPL_ENDOFWHOWAS = 369, //Format: <nick> :<info>
		/*Reply to INFO*/
		RPL_INFO = 371, //Format: :<string>
		/*Reply to MOTD*/
		RPL_MOTD = 372, //Format: :- <string>
		/**/
		RPL_INFOSTART = 373, //Format: 
		/*Termination of an RPL_INFO list*/
		RPL_ENDOFINFO = 374, //Format: :<info>
		/*Start of an RPL_MOTD list*/
		RPL_MOTDSTART = 375, //Format: :- <server> Message of the day -
		/*Termination of an RPL_MOTD list*/
		RPL_ENDOFMOTD = 376, //Format: :<info>
		/*Successful reply from OPER*/
		RPL_YOUREOPER = 381, //Format: :<info>
		/*Successful reply from REHASH*/
		RPL_REHASHING = 382, //Format: <config_file> :<info>
		/*Sent upon successful registration of a service*/
		RPL_YOURESERVICE = 383, //Format: :You are service <service_name>
		/**/
		RPL_MYPORTIS = 384, //Format: 
		/*Response to the TIME command. The string format may vary greatly. Also see #679.*/
		RPL_TIME = 391, //Format: <server> :<time string>
		/*Start of an RPL_USERS list*/
		RPL_USERSSTART = 392, //Format: :UserID   Terminal  Host
		/*Response to the USERS command (See RFC)*/
		RPL_USERS = 393, //Format: :<username> <ttyline> <hostname>
		/*Termination of an RPL_USERS list*/
		RPL_ENDOFUSERS = 394, //Format: :<info>
		/*Reply to USERS when nobody is logged in*/
		RPL_NOUSERS = 395, //Format: :<info>
		/*Used to indicate the nickname parameter supplied to a command is currently unused*/
		ERR_NOSUCHNICK = 401, //Format: <nick> :<reason>
		/*Used to indicate the server name given currently doesn't exist*/
		ERR_NOSUCHSERVER = 402, //Format: <server> :<reason>
		/*Used to indicate the given channel name is invalid, or does not exist*/
		ERR_NOSUCHCHANNEL = 403, //Format: <channel> :<reason>
		/*Sent to a user who does not have the rights to send a message to a channel*/
		ERR_CANNOTSENDTOCHAN = 404, //Format: <channel> :<reason>
		/*Sent to a user when they have joined the maximum number of allowed channels and they tried to join another channel*/
		ERR_TOOMANYCHANNELS = 405, //Format: <channel> :<reason>
		/*Returned by WHOWAS to indicate there was no history information for a given nickname*/
		ERR_WASNOSUCHNICK = 406, //Format: <nick> :<reason>
		/*The given target(s) for a command are ambiguous in that they relate to too many targets*/
		ERR_TOOMANYTARGETS = 407, //Format: <target> :<reason>
		/*Returned to a client which is attempting to send an SQUERY (or other message) to a service which does not exist*/
		ERR_NOSUCHSERVICE = 408, //Format: <service_name> :<reason>
		/*PING or PONG message missing the originator parameter which is required since these commands must work without valid prefixes*/
		ERR_NOORIGIN = 409, //Format: :<reason>
		/*Returned when no recipient is given with a command*/
		ERR_NORECIPIENT = 411, //Format: :<reason>
		/*Returned when NOTICE/PRIVMSG is used with no message given*/
		ERR_NOTEXTTOSEND = 412, //Format: :<reason>
		/*Used when a message is being sent to a mask without being limited to a top-level domain (i.e. * instead of *.au)*/
		ERR_NOTOPLEVEL = 413, //Format: <mask> :<reason>
		/*Used when a message is being sent to a mask with a wild-card for a top level domain (i.e. *.*)*/
		ERR_WILDTOPLEVEL = 414, //Format: <mask> :<reason>
		/*Used when a message is being sent to a mask with an invalid syntax*/
		ERR_BADMASK = 415, //Format: <mask> :<reason>
		/*Returned when the given command is unknown to the server (or hidden because of lack of access rights)*/
		ERR_UNKNOWNCOMMAND = 421, //Format: <command> :<reason>
		/*Sent when there is no MOTD to send the client*/
		ERR_NOMOTD = 422, //Format: :<reason>
		/*Returned by a server in response to an ADMIN request when no information is available. RFC1459 mentions this in the list of numerics. While it's not listed as a valid reply in section 4.3.7 ('Admin command'), it's confirmed to exist in the real world.*/
		ERR_NOADMININFO = 423, //Format: <server> :<reason>
		/*Generic error message used to report a failed file operation during the processing of a command*/
		ERR_FILEERROR = 424, //Format: :<reason>
		/*Returned when a nickname parameter expected for a command isn't found*/
		ERR_NONICKNAMEGIVEN = 431, //Format: :<reason>
		/*Returned after receiving a NICK message which contains a nickname which is considered invalid, such as it's reserved ('anonymous') or contains characters considered invalid for nicknames. This numeric is misspelt, but remains with this name for historical reasons :)*/
		ERR_ERRONEUSNICKNAME = 432, //Format: <nick> :<reason>
		/*Returned by the NICK command when the given nickname is already in use*/
		ERR_NICKNAMEINUSE = 433, //Format: <nick> :<reason>
		/*Returned by a server to a client when it detects a nickname collision*/
		ERR_NICKCOLLISION = 436, //Format: <nick> :<reason>
		/*Return when the target is unable to be reached temporarily, eg. a delay mechanism in play, or a service being offline*/
		ERR_UNAVAILRESOURCE = 437, //Format: <nick/channel/service> :<reason>
		/*Returned by the server to indicate that the target user of the command is not on the given channel*/
		ERR_USERNOTINCHANNEL = 441, //Format: <nick> <channel> :<reason>
		/*Returned by the server whenever a client tries to perform a channel effecting command for which the client is not a member*/
		ERR_NOTONCHANNEL = 442, //Format: <channel> :<reason>
		/*Returned when a client tries to invite a user to a channel they're already on*/
		ERR_USERONCHANNEL = 443, //Format: <nick> <channel> [:<reason>]
		/*Returned by the SUMMON command if a given user was not logged in and could not be summoned*/
		ERR_NOLOGIN = 444, //Format: <user> :<reason>
		/*Returned by SUMMON when it has been disabled or not implemented*/
		ERR_SUMMONDISABLED = 445, //Format: :<reason>
		/*Returned by USERS when it has been disabled or not implemented*/
		ERR_USERSDISABLED = 446, //Format: :<reason>
		/*Returned by the server to indicate that the client must be registered before the server will allow it to be parsed in detail*/
		ERR_NOTREGISTERED = 451, //Format: :<reason>
		/*Returned by the server by any command which requires more parameters than the number of parameters given*/
		ERR_NEEDMOREPARAMS = 461, //Format: <command> :<reason>
		/*Returned by the server to any link which attempts to register again*/
		ERR_ALREADYREGISTERED = 462, //Format: :<reason>
		/*Returned to a client which attempts to register with a server which has been configured to refuse connections from the client's host*/
		ERR_NOPERMFORHOST = 463, //Format: :<reason>
		/*Returned by the PASS command to indicate the given password was required and was either not given or was incorrect*/
		ERR_PASSWDMISMATCH = 464, //Format: :<reason>
		/*Returned to a client after an attempt to register on a server configured to ban connections from that client*/
		ERR_YOUREBANNEDCREEP = 465, //Format: :<reason>
		/*Sent by a server to a user to inform that access to the server will soon be denied*/
		ERR_YOUWILLBEBANNED = 466, //Format: 
		/*Returned when the channel key for a channel has already been set*/
		ERR_KEYSET = 467, //Format: <channel> :<reason>
		/*Returned when attempting to join a channel which is set +l and is already full*/
		ERR_CHANNELISFULL = 471, //Format: <channel> :<reason>
		/*Returned when a given mode is unknown*/
		ERR_UNKNOWNMODE = 472, //Format: <char> :<reason>
		/*Returned when attempting to join a channel which is invite only without an invitation*/
		ERR_INVITEONLYCHAN = 473, //Format: <channel> :<reason>
		/*Returned when attempting to join a channel a user is banned from*/
		ERR_BANNEDFROMCHAN = 474, //Format: <channel> :<reason>
		/*Returned when attempting to join a key-locked channel either without a key or with the wrong key*/
		ERR_BADCHANNELKEY = 475, //Format: <channel> :<reason>
		/*The given channel mask was invalid*/
		ERR_BADCHANMASK = 476, //Format: <channel> :<reason>
		/*Returned when attempting to set a mode on a channel which does not support channel modes, or channel mode changes. Also known as ERR_MODELESS*/
		ERR_NOCHANMODES = 477, //Format: <channel> :<reason>
		/*Returned when a channel access list (i.e. ban list etc) is full and cannot be added to*/
		ERR_BANLISTFULL = 478, //Format: <channel> <char> :<reason>
		/*Returned by any command requiring special privileges (eg. IRC operator) to indicate the operation was unsuccessful*/
		ERR_NOPRIVILEGES = 481, //Format: :<reason>
		/*Returned by any command requiring special channel privileges (eg. channel operator) to indicate the operation was unsuccessful*/
		ERR_CHANOPRIVSNEEDED = 482, //Format: <channel> :<reason>
		/*Returned by KILL to anyone who tries to kill a server*/
		ERR_CANTKILLSERVER = 483, //Format: :<reason>
		/*Sent by the server to a user upon connection to indicate the restricted nature of the connection (i.e. usermode +r)*/
		ERR_RESTRICTED = 484, //Format: :<reason>
		/*Any mode requiring 'channel creator' privileges returns this error if the client is attempting to use it while not a channel creator on the given channel*/
		ERR_UNIQOPRIVSNEEDED = 485, //Format: :<reason>
		/*Returned by OPER to a client who cannot become an IRC operator because the server has been configured to disallow the client's host*/
		ERR_NOOPERHOST = 491, //Format: :<reason>
		/**/
		ERR_NOSERVICEHOST = 492, //Format: 
		/*Returned by the server to indicate that a MODE message was sent with a nickname parameter and that the mode flag sent was not recognised*/
		ERR_UMODEUNKNOWNFLAG = 501, //Format: :<reason>
		/*Error sent to any user trying to view or change the user mode for a user other than themselves*/
		ERR_USERSDONTMATCH = 502, //Format: :<reason>
	};
	extern std::unordered_map<irc::numeric_reply,std::string> const validNumericReplies;
}
