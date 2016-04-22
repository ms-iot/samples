'use strict';

goog.require('Blockly.JavaScript');

Blockly.JavaScript['device_forever'] = function(block) {
    // Do while(true) loop.
    var branch = Blockly.JavaScript.statementToCode(block, 'DO');
    branch = Blockly.JavaScript.addLoopTrap(branch, block.id);
    return 'while (true) {\n' + branch + '}\n';
};

Blockly.JavaScript['device_pause'] = function(block) {
    // Pause statement.
    var pause = Blockly.JavaScript.valueToCode(block, 'PAUSE', Blockly.JavaScript.ORDER_ASSIGNMENT) || '100';
    return 'basic.pause(' + pause + ');\n';
};

Blockly.JavaScript['device_print_message'] = function(block) {
    // print statement.
    var msg = Blockly.JavaScript.valueToCode(block, 'MESSAGE', Blockly.JavaScript.ORDER_ASSIGNMENT) || 'Hello!';
    return 'senseHat.print(' + msg + ', Windows.UI.Colors.black, Windows.UI.Colors.blue);\n';
};

Blockly.JavaScript['device_show_number'] = function(block) {
    // print statement.
    var msg = Blockly.JavaScript.valueToCode(block, 'NUMBER', Blockly.JavaScript.ORDER_ASSIGNMENT) || '2';
    return 'senseHat.print(' + msg + ', Windows.UI.Colors.black, Windows.UI.Colors.blue);\n';
};

Blockly.JavaScript['device_show_leds'] = function(block) {
    // show leds on the screen.
    var color = String(block.getFieldValue('COLOR'));
    var matrix = '';
    for (var y = 0; y < 8; ++y) {
        for (var x = 0; x < 8; ++x) {
            if (block.getFieldValue('LED' + x + y) === 'TRUE') {
                matrix += '1';
            } else {
                matrix += '0';
            }
        }
        matrix += '|';
    }
    return 'senseHat.drawLedMatrix(\'' + matrix + '\', ' + color + ', 0, 0, true);\n';
};

Blockly.JavaScript['device_clear_display'] = function(block) {
    // Clear screen
    return 'senseHat.clear(true);\n';
};

Blockly.JavaScript['device_clear_display_no_update'] = function(block) {
    // Clear screen without refreshing the screen
    return 'senseHat.clear(false);\n';
};

Blockly.JavaScript['device_set_onboard_led'] = function(block) {
    // Turn LED on/off statement.
    var state = String(Number(block.getFieldValue('STATE')));
    return 'gpio.setOnboardLED(' + state + ');\n';
};

Blockly.JavaScript['device_digital_write_pin'] = function(block) {
    // Turn a specific a GPIO pin on or off.
    var value = Blockly.JavaScript.valueToCode(block, 'VALUE', Blockly.JavaScript.ORDER_ASSIGNMENT) || '4';
    var pin = Blockly.JavaScript.valueToCode(block, 'PIN', Blockly.JavaScript.ORDER_ASSIGNMENT) || '4';
    return 'gpio.setGPIOPin(' + pin + ", " + value + ');\n';
};

Blockly.JavaScript['device_random'] = function(block) {
    // Return a random number
    var limit = Blockly.JavaScript.valueToCode(block, 'LIMIT', Blockly.JavaScript.ORDER_ASSIGNMENT) || '4';
    var code = 'Math.floor(Math.random() * ((' + limit + ') + 1))';
    return [code, Blockly.JavaScript.ORDER_ATOMIC];
};

Blockly.JavaScript['device_get_acceleration'] = function(block) {
    // Return the acceleration on an axis (between -2048 and 2047)
    var axis = String(Number(block.getFieldValue('AXIS')));
    var code = 'senseHat.getAccel(' + axis + ')';
    return [code, Blockly.JavaScript.ORDER_ATOMIC];
};

Blockly.JavaScript['device_create_image'] = function(block) {
    // Image value (sprite) 
    var matrix = '';
    for (var y = 0; y < 8; ++y) {
        for (var x = 0; x < 8; ++x) {
            if (block.getFieldValue('LED' + x + y) === 'TRUE') {
                matrix += '1';
            } else {
                matrix += '0';
            }
        }
        matrix += '|';
    }
    var code = Blockly.JavaScript.quote_(matrix);
    return [code, Blockly.JavaScript.ORDER_ATOMIC];
};

Blockly.JavaScript['device_create_small_image'] = function(block) {
    // Image value (sprite) 
    var matrix = '';
    for (var y = 0; y < 4; ++y) {
        for (var x = 0; x < 4; ++x) {
            if (block.getFieldValue('LED' + x + y) === 'TRUE') {
                matrix += '1';
            } else {
                matrix += '0';
            }
        }
        matrix += '|';
    }
    var code = Blockly.JavaScript.quote_(matrix);
    return [code, Blockly.JavaScript.ORDER_ATOMIC];
};

Blockly.JavaScript['device_show_image_offset'] = function(block) {
    // show leds on the screen.
    var matrix = Blockly.JavaScript.valueToCode(block, 'SPRITE', Blockly.JavaScript.ORDER_ASSIGNMENT) || '';
    var color = String(block.getFieldValue('COLOR'));
    var offsetX = Blockly.JavaScript.valueToCode(block, 'OFFSETX', Blockly.JavaScript.ORDER_ASSIGNMENT) || '0';
    var offsetY = Blockly.JavaScript.valueToCode(block, 'OFFSETY', Blockly.JavaScript.ORDER_ASSIGNMENT) || '0';
    return 'senseHat.drawLedMatrix(' + matrix + ', ' + color + ', ' + offsetX + ', ' + offsetY + ', true);\n';
};
