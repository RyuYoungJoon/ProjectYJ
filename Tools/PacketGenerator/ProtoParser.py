

class ProtoParser():
    def __init__(self, start_type, request_suffix, ack_suffix):
        self.request_packet = []; # ���� ��Ŷ ���
        self.ack_packet = []; # �۽� ��Ŷ ���
        self.total_packet = []; # ��� ��Ŷ ���
        self.start_type = start_type
        self.type = start_type
        self.request_suffix = request_suffix
        self.ack_suffix = ack_suffix

    def parse_proto(self, path):
        f = open(path, 'r')
        lines = f.readlines()

        for line in lines:
            if line.startswith('message') == False:
                continue

            packet_name = line.split()[1]
            if packet_name.endswith(self.request_suffix):
                self.request_packet.append(Packet(packet_name, self.type))
            elif packet_name.endswith(self.ack_suffix):
                self.ack_packet.append(Packet(packet_name, self.type))
            else:
                continue

            self.total_packet.append(Packet(packet_name, self.type))
            self.type += 1

        f.close()
            


class Packet:
    def __init__(self, name, type):
        self.name = name
        self.type = type

