import socket

from .protocol import Transport, Message, Packet

class Socket:
	def __init__(self, sock=None):
		if sock is None:
			sock = socket.socket(socket.AF_UNIX)
			sock.connect("/var/run/charon.vici")
		self.handler = SessionHandler(Transport(sock))
	
	def version(self):
		return self.handler.request("version");
	
	def initiate(self, sa):
		return self.handler.stream_request("initiate", "control-log", sa)
	
class SessionHandler(object):
	def __init__(self, transport):
		self.transport = transport
	
	def _communicate(self, packet):
		self.transport.send(packet)
		return Packet.parse(self.transport.receive())
	
	def _register_unregister(self, event_type, register):
		if register:
			packet = Packet.register_event(event_type)
		else:
			packet = Packet.unregister_event(event_type)
		response = self._communicate(packet)
		if response.response_type == Packet.EVENT_UNKNOWN:
			raise EventUnknownException(
				"Unknown event type '{event}'".format(event=event_type)
			)
		elif response.response_type != Packet.EVENT_CONFIRM:
			raise SessionException(
				"Unexpected response type {type}, "
				"excepted '{confirm}' (EVENT_CONFIRM)".format(
					type=response.response_type,
					confirm=Packet.EVENT_CONFIRM,
				)
			)
	
	def streamed_request(self, command, event_stream_type, message=None):
		if message is not None:
			message = Message.serialize(message)
		
		self._register_unregister(event_stream_type, True)
		
		try:
			packet = Packet.request(command, message)
			self.transport.send(packet)
			exited = False
			while True:
				response = Packet.parse(self.transport.receive())
				if response.response_type == PACKET.EVENT:
					if not exited:
						try:
							yield Message.deserialize(response.payload)
						except GeneratorExit:
							exited = True
							pass
				else:
					break
			if response.response_type == Packet.CMD_RESPONSE:
				command_response = Message.deserialize(response.payload)	
		finally:
			self._register_unregister(event_stream_type, False)
