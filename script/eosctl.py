#!/bin/env python3

from urllib.request import urlopen
import socket, os, argparse

ADDRESS = "192.168.77.1"
SLS_PORT = 9002
GDB_PORT = 1234
HTTP_PORT = 80
CMD_LENGTH_LIMIT = 64
PAYLOAD_LENGTHLIMIT = 1306
MAX_UPLOAD_FRAME_SIZE = PAYLOAD_LENGTHLIMIT - 6
SLS_ADDRESS = (ADDRESS, SLS_PORT)
GDB_ADDRESS = (ADDRESS, GDB_PORT)
HTTP_ADDRESS = (ADDRESS, HTTP_PORT)
CLIENT_TIMEOUT = 1.0

class SideLoadClient:
    def __init__(self, sls_addr=SLS_ADDRESS, gdb_addr=GDB_ADDRESS, http_addr=HTTP_ADDRESS, max_recv_buffer_size=1024):
        self.sls_addr = sls_addr
        self.gdb_addr = gdb_addr
        self.http_addr = http_addr
        self.udp = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.udp.settimeout(CLIENT_TIMEOUT)
        self.bf_size = max_recv_buffer_size

    def __del__(self):
        self.udp.close()

    @staticmethod
    def __gdb_cmd(sock: socket.socket, cmd: str) -> None:
        checksum = sum(cmd.encode("utf-8")) % 256
        sock.sendall(f"${cmd}#{checksum:02X}".encode("utf-8"))

    @staticmethod
    def __gdb_recv(sock: socket.socket) -> str:
        checksum = 0
        data = bytearray()
        end_count = -1
        while True:
            b = sock.recv(1)[0]
            data.append(b)
            if end_count >= 0:
                end_count += 1
            if b == b"#"[0]:
                end_count = 0
            if end_count >= 2:
                break
        # ignore checksum
        return data.decode("utf-8")[2:-3]

    @staticmethod
    def __to_cstring(text: str) -> bytes:
        return text.encode("utf-8") + b"\0"

    @staticmethod
    def __is_resp_success(resp: bytes) -> bool:
        return len(resp) > 0 and resp[0] == b"T"[0]

    def __send(self, data: bytes) -> None:
        self.udp.sendto(data, self.sls_addr)

    def __recv_result(self):
        resp, addr = self.udp.recvfrom(self.bf_size)
        if addr == self.sls_addr:
            return SideLoadClient.__is_resp_success(resp)
        return False

    def _upload_start(self, filename: str) -> bool:
        cmd = f"UPLOAD_{filename}"
        data = SideLoadClient.__to_cstring(cmd)
        assert len(data) <= CMD_LENGTH_LIMIT
        self.__send(data)
        return self.__recv_result()

    def _upload_write(self, udata: bytes) -> bool:
        size = len(udata)
        data = b"U_" + f"{size:04d}".encode("utf-8") + udata
        assert len(data) <= PAYLOAD_LENGTHLIMIT
        self.__send(data)
        return self.__recv_result()

    def _upload_done(self) -> bool:
        self.__send("UDONE".encode("utf-8"))
        return self.__recv_result()

    def upload_file(self, file_path: str, upload_name: str = None) -> bool:
        if not os.path.exists(file_path):
            return False
        if not upload_name:
            upload_name = os.path.basename(file_path)
        assert self._upload_start(upload_name)
        try:
            f_size = os.path.getsize(file_path)
            with open(file_path, "rb") as local_file:
                data = local_file.read(MAX_UPLOAD_FRAME_SIZE)
                write_size = len(data)
                while len(data) > 0:
                    assert self._upload_write(data)
                    print(f":UPLOAD {write_size}/{f_size}")
                    data = local_file.read(MAX_UPLOAD_FRAME_SIZE)
                    write_size += len(data)
        finally:
            assert self._upload_done()
        return True

    def kill(self) -> bool:
        self.__send("KILL".encode("utf-8"))
        return self.__recv_result()

    # def restart_side_load_server(self) -> bool:
    #     self.__send("SLS_START".encode("utf-8"))
    #     return self.__recv_result()

    def execute(self, filename: str) -> bool:
        cmd = f"EXEC_{filename}"
        data = SideLoadClient.__to_cstring(cmd)
        assert len(data) <= CMD_LENGTH_LIMIT
        self.__send(data)
        return self.__recv_result()

    def reset(self) -> None:
        try:
            urlopen(f"http://{self.http_addr[0]}:{self.http_addr[1]}/reset", timeout=0.1)
        except TimeoutError:
            pass
        # ignore response

    def get_std_out(self) -> bytes:
        resp = urlopen(f"http://{self.http_addr[0]}:{self.http_addr[1]}/getStdout", timeout=CLIENT_TIMEOUT)
        # print(resp.read().decode(errors='backslashreplace'))
        return resp.read()



def __args():
    parser = argparse.ArgumentParser()
    subparsers = parser.add_subparsers(
        title="subcommands",
        required=True,
    )
    # subcommand upload
    p_upload = subparsers.add_parser(
        "upload",
        help="Upload a file",
    )
    p_upload.add_argument(
        "local_file",
        metavar="LOCAL_FILE",
        help="File to upload",
    )
    p_upload.add_argument(
        "-n", "--name",
        metavar="REMOTE_NAME",
        default="",
        help="Remote name",
    )
    p_upload.set_defaults(subcmd="upload")
    # subcommand kill
    p_kill = subparsers.add_parser(
        "kill",
        help="Stop current running app",
    )
    p_kill.set_defaults(subcmd="kill")
    # subcommand exec
    p_exec = subparsers.add_parser(
        "exec",
        help="Run a .exp app",
    )
    p_exec.add_argument(
        "remote_app",
        metavar="REMOTE_APP",
        help=".exp file to execute",
    )
    p_exec.set_defaults(subcmd="exec")
    # subcommand reset
    p_reset = subparsers.add_parser(
        "reset",
        help="Reset remote machine",
    )
    p_reset.set_defaults(subcmd="reset")
    # subcommand upload_exec
    p_upload_exec = subparsers.add_parser(
        "upload_exec",
        help="Upload and execute a file",
    )
    p_upload_exec.add_argument(
        "local_file",
        metavar="LOCAL_FILE",
        help="File to upload",
    )
    p_upload_exec.add_argument(
        "-n", "--name",
        metavar="REMOTE_NAME",
        default="",
        help="Remote name",
    )
    p_upload_exec.set_defaults(subcmd="upload_exec")
    # subcommand monitor
    p_monitor = subparsers.add_parser(
        "monitor",
        help="Monitor for output",
    )
    p_monitor.set_defaults(subcmd="monitor")

    args = parser.parse_args()
    return args

def __main():
    args = __args()
    client = SideLoadClient()
    if (args.subcmd in ["upload", "upload_exec"]):
        if len(args.name.strip()) <= 0:
            rname = os.path.basename(args.local_file)
        else:
            rname = args.name
        client.upload_file(args.local_file, rname)
        if args.subcmd == "upload_exec":
            client.execute(rname)
    if (args.subcmd == "kill"):
        client.kill()
    if (args.subcmd == "reset"):
        client.reset()
    if (args.subcmd == "exec"):
        client.execute(args.remote_app)
    if (args.subcmd == "monitor"):
        try:
            import time
            while True:
                try:
                    output = client.get_std_out().decode("utf-8", errors='backslashreplace')
                except KeyboardInterrupt:
                    raise
                except:
                    time.sleep(1.0)
                    continue
                if len(output) > 0:
                    print(output, end="")
                time.sleep(0.5)
        except KeyboardInterrupt:
            # res = client.kill()
            pass


if __name__ == "__main__":
    __main()
