import _wini2c as i2c

class SMBus:
    """
    SMBus([bus]) -> SMBus

    Return a new SMBus object that is (optionally) connected to the
    specified I2C device interface.
    """

    combine_write_read = False
    """
    By default, raspbian doesn't use "repeated start" functionality.  See 
    https://www.raspberrypi.org/forums/viewtopic.php?f=44&t=15840&start=25 and
    http://ciaduck.blogspot.com/2014/12/mpl3115a2-sensor-with-raspberry-pi.html
    for more details.

    Set to 'False' to simulate raspbian behavior.  Set to 'True' to use 
    "repeated start" functionality.
    """

    _block_max = 32
    _endianness = 'little'

    def __init__(self, bus_id=-1):
        self._bus_id = bus_id
        self._address = -1
        self._i2c_device = None

    def open(self, bus_id):
        """
        open(bus)

        Connects the object to the specified SMBus.
        """
        self._bus_id = bus_id

    def close(self):
        """
        close()

        Disconnects the object from the bus.
        """
        self._bus_id = -1
        self._address = -1
        self._i2c_device = None

    def read_byte(self, address):
        """
        read_byte(address) -> result

        Perform SMBus Read Byte transaction.
        """
        buf = self.__read(address, 1)
        return buf[0]

    def write_byte(self, address, value):
        """
        write_byte(address, value)

        Perform SMBus Write Byte transaction.
        """
        if (not isinstance(value, int)):
            raise TypeError("value must be an integer")

        self.__write(address, value.to_bytes(1, SMBus._endianness))

    def read_byte_data(self, address, command):
        """
        read_byte_data(address, command) -> result

        Perform SMBus Read Byte Data transaction.
        """
        if (not isinstance(command, int)):
            raise TypeError("command must be an integer")

        wbuf = command.to_bytes(1, SMBus._endianness)
        return self.__write_read(address, wbuf, 1)

    def write_byte_data(self, address, command, value):
        """
        write_byte_data(address, command, value)

        Perform SMBus Write Byte Data transaction.
        """
        if (not isinstance(command, int)):
            raise TypeError("command must be an integer")
        if (not isinstance(value, int)):
            raise TypeError("value must be an integer")

        buf = command.to_bytes(1, SMBus._endianness) + value.to_bytes(1, SMBus._endianness)
        self.__write(address, buf)

    def read_word_data(self, address, command):
        """
        read_word_data(address, command) -> result

        Perform SMBus Read Word Data transaction.
        """
        if (not isinstance(command, int)):
            raise TypeError("command must be an integer")

        wbuf = command.to_bytes(1, SMBus._endianness)
        buf = self.__write_read(address, wbuf, 2)
        return int.from_bytes(buf, byteorder=SMBus._endianness)

    def write_word_data(self, address, command, word):
        """
        write_word_data(address, command, value)

        Perform SMBus Write Word Data transaction.
        """
        if (not isinstance(command, int)):
            raise TypeError("command must be an integer")
        if (not isinstance(word, int)):
            raise TypeError("word must be an integer")

        buf = command.to_bytes(1, SMBus._endianness) + word.to_bytes(2, SMBus._endianness)
        self.__write(address, buf)

    def process_call(self, address, command, word):
        """
        process_call(address, command, value)

        Perform SMBus Process Call transaction.
        """
        if (not isinstance(command, int)):
            raise TypeError("command must be an integer")
        if (not isinstance(word, int)):
            raise TypeError("word must be an integer")

        wbuf = command.to_bytes(1, SMBus._endianness) + word.to_bytes(2, SMBus._endianness)
        self.__write_read(address, wbuf, 2)

    def read_block_data(self, address, command):
        """
        read_block_data(address, command) -> results

        Perform SMBus Read Block Data transaction.
        """
        if (not isinstance(command, int)):
            raise TypeError("command must be an integer")

        wbuf = command.to_bytes(1, SMBus._endianness)
        buf = self.__write_read_partial(address, wbuf, SMBus._block_max)
        length = buf[0]
        return list(buf[1:1+length])

    def write_block_data(self, address, command, values):
        """
        write_block_data(address, command, [values])

        Perform SMBus Write Block Data transaction.
        """
        if (not isinstance(command, int)):
            raise TypeError("command must be an integer")
        if (not isinstance(values, list)):
            raise TypeError("values must be a list")

        length = len(values)
        if length > SMBus._block_max:
            raise OverflowError("Third argument must be a list not more than %d integers" % SMBus._block_max)

        wbuf = command.to_bytes(1, SMBus._endianness) + length.to_bytes(1, SMBus._endianness) + bytes(values)
        self.__write(address, wbuf)

    def block_process_call(self, address, command, values):
        """
        block_process_call(address, command, [values]) -> results

        Perform SMBus Block Process Call transaction.
        """
        if (not isinstance(command, int)):
            raise TypeError("command must be an integer")
        if (not isinstance(values, list)):
            raise TypeError("values must be a list")

        length = len(values)
        if length > SMBus._block_max:
            raise OverflowError("Third argument must be a list of not more than %d integers" % SMBus._block_max)

        wbuf = command.to_bytes(1, SMBus._endianness) + word.to_bytes(2, SMBus._endianness) + length.to_bytes(1, SMBus._endianness) + bytes(values)
        rbuf = self_write_read_partial(wbuf, SMBus._block_max)
        length = rbuf[0]
        return list(buf[1:1+length])

    def read_i2c_block_data(self, address, command):
        """
        read_i2c_block_data(address, command) -> results

        Perform I2C Block Read transaction.
        """
        if (not isinstance(command, int)):
            raise TypeError("command must be an integer")

        return list(self.__write_read(address, command.to_bytes(1, SMBus._endianness), SMBus._block_max))

    def write_i2c_block_data(self, address, command, block):
        """
        write_i2c_block_data(address, command, block)

        Perform I2C Block Write transaction.
        """
        if (not isinstance(command, int)):
            raise TypeError("command must be an integer")
        if (not isinstance(block, list)):
            raise TypeError("block must be a list")

        if len(block) > SMBus._block_max:
            raise OverflowError("Third argument must be a list of not more than %d integers" % SMBus._block_max)

        buf = command.to_bytes(1, SMBus._endianness) + bytes(block)
        self.__write(address, buf)

    def __get_i2c_device(self, address):
        if (not isinstance(address, int)):
                raise TypeError("address must be an integer")
        if (self._address != address):
            self._i2c_device = i2c.i2cdevice(self._bus_id - 1, address, i2c.STANDARDSPEED, i2c.EXCLUSIVEMODE)
            self._address = address
        return self._i2c_device

    def __read(self, address, length):
        try:
            i2c_device = self.__get_i2c_device(address)
            return i2c_device.read(length)
        except RuntimeError as e:
            raise IOError() from e

    def __read_partial(self, address, length):
        try:
            i2c_device = self.__get_i2c_device(address)
            return i2c_device.read_partial(length)
        except RuntimeError as e:
            raise IOError() from e

    def __write(self, address, buf):
        try:
            i2c_device = self.__get_i2c_device(address)
            return i2c_device.write(buf)
        except RuntimeError as e:
            raise IOError() from e

    def __write_read(self, address, wbuf, read_len):
        try:
            i2c_device = self.__get_i2c_device(address)
            if SMBus.combine_write_read == True:
                return i2c_device.writeread(wbuf, read_len)
            else:
                i2c_device.write(wbuf)
                return i2c_device.read(read_len)
        except RuntimeError as e:
            raise IOError() from e

    def __write_read_partial(self, address, wbuf, read_len):
        try:
            i2c_device = self.__get_i2c_device(address)
            if SMBus.combine_write_read == True:
                return i2c_device.writeread_partial(wbuf, read_len)
            else:
                i2c_device.write(wbuf)
                return i2c_device.read_partial(read_len)
        except RuntimeError as e:
            raise IOError() from e
