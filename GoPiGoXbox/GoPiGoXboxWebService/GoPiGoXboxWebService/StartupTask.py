from flask import Flask
from flask.ext.restful import Api, Resource, reqparse

import gopigo
# Set some initial parameters
gopigo.enable_com_timeout(1000)
gopigo.stop()

app = Flask(__name__, static_url_path="")
api = Api(app)

class GamePadAPI(Resource):

    _button_menu = 1
    _button_view = 2
    _button_A = 4
    _button_B = 8
    _button_X = 16
    _button_Y = 32
    _button_dpad_up = 64
    _button_dpad_down = 128
    _button_dpad_left = 256
    _button_dpad_right = 512
    _button_left_shoulder = 1024
    _button_right_shoulder = 2048
    _button_left_thumbstick = 4096
    _button_right_thumbstick = 8192

    def __init__(self):
        self.reqparse = reqparse.RequestParser()
        self.reqparse.add_argument('LeftThumbstickX', type=float, required=True, help='No LeftThumbstickX provided', location='json')
        self.reqparse.add_argument('LeftThumbstickY', type=float, required=True, help="No LeftThumbstickY provided", location='json')
        self.reqparse.add_argument('LeftTrigger', type=float, required=True, help="No LeftTrigger provided", location='json')
        self.reqparse.add_argument('RightThumbstickX', type=float, required=True, help='No RightThumbstickX provided', location='json')
        self.reqparse.add_argument('RightThumbstickY', type=float, required=True, help="No RightThumbstickY provided", location='json')
        self.reqparse.add_argument('RightTrigger', type=float, required=True, help="No RightTrigger provided", location='json')
        self.reqparse.add_argument('Buttons', type=int, required=True, help="No Buttons provided", location='json')
        super(GamePadAPI, self).__init__()

    def get(self):
        return {"Result":"Ok"}, 200

    def post(self):
        data = self.reqparse.parse_args()
        #print(data)         # uncomment this line to see the raw joystick input

        if not self._process_left_thumbstick(data) and not self._process_dpad(data):
            # No input was given, stop the motor
            gopigo.stop()

        return {}, 200

    def _process_dpad(self, data):
        """
        Read the state of the dpad buttons and issue the corresponding command
        to GoPiGo.  If no dpad buttons were pressed, return False.  Return
        True otherwise.
        """
        if data['Buttons'] & self._button_dpad_up:
            # move forward
            gopigo.motor_fwd()
        elif data['Buttons'] & self._button_dpad_down:
            # move backward
            gopigo.motor_bwd()
        elif data['Buttons'] & self._button_dpad_left:
            # rotate left
            gopigo.left_rot()
        elif data['Buttons'] & self._button_dpad_right:
            # rotate right
            gopigo.right_rot()
        else:
            # no input
            return False

        return True

    def _process_left_thumbstick(self, data):
        """
        Read the state of the left thumbstick and issue the corresponding command
        to GoPiGo.  If no input were given, return False.  Return
        True otherwise.
        """
        
        # The valid values ranges from -1.0 to 1.0.  Thus, setting threshold
        # at 0.8.
        threshold = 0.8

        # Read the Left thumb stick
        X = data['LeftThumbstickX']
        Y = data['LeftThumbstickY']

        # Take the absolute value
        abs_X = abs(X)
        abs_Y = abs(Y)

        if abs_X > abs_Y and abs_X > threshold:
            if X > 0:
                # rotate right
                gopigo.right_rot()
            else:
                # rotate left
                gopigo.left_rot()
        elif abs_Y > abs_X and abs_Y > threshold:
            if Y > 0:
                # move forward
                gopigo.motor_fwd()
            else:
                # move backward
                gopigo.motor_bwd()
        else:
            # No Input
            return False

        return True


api.add_resource(GamePadAPI, '/gamepad', endpoint='gamepad')

if __name__ == '__main__':
    app.run(host='0.0.0.0', debug=False, use_reloader=False)
