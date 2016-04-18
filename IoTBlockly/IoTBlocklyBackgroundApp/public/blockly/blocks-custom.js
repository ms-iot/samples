'use strict';

goog.provide('Blockly.Blocks.device');

goog.require('Blockly.Blocks');

var blockColors = {
    basic: 190,
    led: 300,
    input: 3,
    loops: 120,
    pins: 351,
    music: 52,
    game: 176,
    comments: 156,
    images:45,
}

var ledStateDropdown = [
  ["on", "1"],
  ["off", "0"]
];

var ledColorDropdown = [
  ["red", "Windows.UI.Colors.red"],
  ["yellow", "Windows.UI.Colors.yellow"],
  ["blue", "Windows.UI.Colors.blue"],
  ["green", "Windows.UI.Colors.green"],
  ["black", "Windows.UI.Colors.black"],
  ["white", "Windows.UI.Colors.white"]
];

var accelAxisDropdown = [
  ["x", "0"],
  ["y", "1"],
  ["z", "2"]
]

Blockly.Blocks['device_pause'] = {
  init: function() {
    this.setHelpUrl('https://www.microbit.co.uk/functions/pause');
    this.setColour(blockColors.basic);
    this.appendDummyInput()
        .appendField("pause (ms)");
    this.appendValueInput("PAUSE")
        .setCheck("Number");
    this.setInputsInline(true);
    this.setPreviousStatement(true);
    this.setNextStatement(true);
    this.setTooltip('Stop execution for the given delay, hence allowing other threads of execution to run.');
  }
};

Blockly.Blocks['device_forever'] = {
  init: function() {
    this.setHelpUrl('https://www.microbit.co.uk/functions/forever');
    this.setColour(blockColors.basic);
    this.appendDummyInput()
        .appendField("forever");
    this.appendStatementInput("DO")
        .setCheck("null");
    this.setInputsInline(true);
    this.setPreviousStatement(true);
    this.setTooltip('Run a sequence of operations repeatedly, in the background.');
  }
};

//https://blockly-demo.appspot.com/static/demos/blockfactory/index.html#nwf7c5
Blockly.Blocks['device_clear_display'] = {
    init: function () {
        this.setHelpUrl('https://www.microbit.co.uk/functions/clear-screen');
        this.setColour(blockColors.basic);
        this.appendDummyInput()
            .appendField("clear screen");
        this.setInputsInline(true);
        this.setPreviousStatement(true);
        this.setNextStatement(true);
        this.setTooltip('Clear screen by turning all LEDs off.');
    }
};

Blockly.Blocks['device_clear_display_no_update'] = {
    init: function () {
        this.setHelpUrl('https://www.microbit.co.uk/functions/clear-screen');
        this.setColour(blockColors.basic);
        this.appendDummyInput()
            .appendField("clear screen (no refresh)");
        this.setInputsInline(true);
        this.setPreviousStatement(true);
        this.setNextStatement(true);
        this.setTooltip('Clear screen by turning all LEDs off, without refreshing the screen.');
    }
};

//https://blockly-demo.appspot.com/static/demos/blockfactory/index.html#xiu9u7
Blockly.Blocks['device_show_number'] = {
    init: function () {
        this.setHelpUrl('https://www.microbit.co.uk/functions/show-number');
        this.setColour(blockColors.basic);
        this.appendDummyInput()
            .appendField("show number");
        this.appendValueInput("NUMBER")
            .setCheck("Number")
            .setAlign(Blockly.ALIGN_RIGHT);
        this.setInputsInline(true);
        this.setPreviousStatement(true);
        this.setNextStatement(true);
        this.setTooltip('Shows the specified number and scrolls it if necessary.');
    }
};

//https://blockly-demo.appspot.com/static/demos/blockfactory/index.html#tmkc86
Blockly.Blocks['device_print_message'] = {
    init: function () {
        this.setHelpUrl('https://www.microbit.co.uk/functions/show-string');
        this.setColour(blockColors.basic);
        this.appendDummyInput()
            .appendField("show");
        this.appendValueInput("MESSAGE")
            .setCheck("String")
            .setAlign(Blockly.ALIGN_RIGHT)
            .appendField("string");
        this.setPreviousStatement(true);
        this.setNextStatement(true);
        this.setTooltip('Shows the specified string and scrolls it if necessary.');
        this.setInputsInline(true);
    }
};

Blockly.Blocks['device_show_leds'] = {
    init: function () {
        this.setColour(blockColors.basic);
        this.setPreviousStatement(true);
        this.setNextStatement(true);
        this.appendDummyInput()
            .appendField("show leds in ")
            .appendField(new Blockly.FieldDropdown(ledColorDropdown), "COLOR");
        this.appendDummyInput().appendField("    0     1     2     3     4     5     6     7");
        this.appendDummyInput().appendField("0").appendField(new Blockly.FieldCheckbox("FALSE"), "LED00").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED10").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED20").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED30").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED40").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED50").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED60").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED70");
        this.appendDummyInput().appendField("1").appendField(new Blockly.FieldCheckbox("FALSE"), "LED01").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED11").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED21").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED31").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED41").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED51").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED61").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED71");
        this.appendDummyInput().appendField("2").appendField(new Blockly.FieldCheckbox("FALSE"), "LED02").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED12").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED22").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED32").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED42").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED52").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED62").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED72");
        this.appendDummyInput().appendField("3").appendField(new Blockly.FieldCheckbox("FALSE"), "LED03").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED13").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED23").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED33").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED43").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED53").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED63").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED73");
        this.appendDummyInput().appendField("4").appendField(new Blockly.FieldCheckbox("FALSE"), "LED04").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED14").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED24").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED34").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED44").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED54").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED64").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED74");
        this.appendDummyInput().appendField("5").appendField(new Blockly.FieldCheckbox("FALSE"), "LED05").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED15").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED25").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED35").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED45").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED55").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED65").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED75");
        this.appendDummyInput().appendField("6").appendField(new Blockly.FieldCheckbox("FALSE"), "LED06").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED16").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED26").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED36").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED46").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED56").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED66").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED76");
        this.appendDummyInput().appendField("7").appendField(new Blockly.FieldCheckbox("FALSE"), "LED07").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED17").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED27").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED37").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED47").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED57").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED67").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED77");
        this.setTooltip('Show the given led pattern on the display.');
        this.setHelpUrl("https://www.microbit.co.uk/functions/show-leds");
    }
};

Blockly.Blocks['device_random'] = {
  init: function() {
    this.setHelpUrl('https://www.microbit.co.uk/blocks/contents');
    this.setColour(230);
    this.appendDummyInput()
        .appendField("pick random 0 to ")
    this.appendValueInput("LIMIT")
        .setCheck("Number");
    this.setInputsInline(true);
    this.setOutput(true, "Number");
    this.setTooltip('Returns a random integer between 0 and the specified bound (inclusive).');
  }
};

Blockly.Blocks['device_create_image'] = {
    init: function () {
        this.setColour(blockColors.images);
        this.appendDummyInput()
            .appendField("create image")
        this.appendDummyInput().appendField("    0     1     2     3     4     5     6     7");
        this.appendDummyInput().appendField("0").appendField(new Blockly.FieldCheckbox("FALSE"), "LED00").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED10").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED20").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED30").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED40").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED50").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED60").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED70");
        this.appendDummyInput().appendField("1").appendField(new Blockly.FieldCheckbox("FALSE"), "LED01").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED11").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED21").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED31").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED41").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED51").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED61").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED71");
        this.appendDummyInput().appendField("2").appendField(new Blockly.FieldCheckbox("FALSE"), "LED02").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED12").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED22").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED32").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED42").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED52").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED62").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED72");
        this.appendDummyInput().appendField("3").appendField(new Blockly.FieldCheckbox("FALSE"), "LED03").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED13").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED23").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED33").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED43").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED53").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED63").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED73");
        this.appendDummyInput().appendField("4").appendField(new Blockly.FieldCheckbox("FALSE"), "LED04").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED14").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED24").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED34").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED44").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED54").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED64").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED74");
        this.appendDummyInput().appendField("5").appendField(new Blockly.FieldCheckbox("FALSE"), "LED05").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED15").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED25").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED35").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED45").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED55").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED65").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED75");
        this.appendDummyInput().appendField("6").appendField(new Blockly.FieldCheckbox("FALSE"), "LED06").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED16").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED26").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED36").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED46").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED56").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED66").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED76");
        this.appendDummyInput().appendField("7").appendField(new Blockly.FieldCheckbox("FALSE"), "LED07").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED17").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED27").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED37").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED47").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED57").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED67").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED77");
        this.setOutput(true, 'sprite');
        this.setTooltip('Create an image with the given led pattern.');
        this.setHelpUrl("https://www.microbit.co.uk/functions/show-leds");
    }
};

Blockly.Blocks['device_create_small_image'] = {
    init: function () {
        this.setColour(blockColors.images);
        this.appendDummyInput()
            .appendField("create small image")
        this.appendDummyInput().appendField("    0     1     2     3");
        this.appendDummyInput().appendField("0").appendField(new Blockly.FieldCheckbox("FALSE"), "LED00").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED10").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED20").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED30");
        this.appendDummyInput().appendField("1").appendField(new Blockly.FieldCheckbox("FALSE"), "LED01").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED11").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED21").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED31");
        this.appendDummyInput().appendField("2").appendField(new Blockly.FieldCheckbox("FALSE"), "LED02").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED12").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED22").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED32");
        this.appendDummyInput().appendField("3").appendField(new Blockly.FieldCheckbox("FALSE"), "LED03").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED13").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED23").appendField(" ").appendField(new Blockly.FieldCheckbox("FALSE"), "LED33");
        this.setOutput(true, 'sprite');
        this.setTooltip('Create a small image with the given led pattern.');
        this.setHelpUrl("https://www.microbit.co.uk/functions/show-leds");
    }
};

Blockly.Blocks['device_show_image_offset'] = {
  init: function() {
    this.setColour(blockColors.images);
    this.appendDummyInput()
        .appendField("show image ");
    this.appendValueInput("SPRITE")
        .setCheck('sprite');
    this.appendDummyInput()
        .appendField("with color ")
        .appendField(new Blockly.FieldDropdown(ledColorDropdown), "COLOR");
    this.appendDummyInput()
        .appendField("at offset ");
    this.appendValueInput("OFFSETX")
        .setCheck("Number")
        .appendField("x ");
    this.appendValueInput("OFFSETY")
        .setCheck("Number")
        .appendField("y ");
    this.setTooltip('Show an image with the selected color at offset (x, y).');
    this.setPreviousStatement(true);
    this.setNextStatement(true);
    this.setInputsInline(true);
    this.setHelpUrl('https://www.microbit.co.uk/functions/show-image');
  }
};

Blockly.Blocks['device_get_acceleration'] = {
    init: function () {
        this.setHelpUrl('https://www.microbit.co.uk/functions/acceleration');
        this.setColour(blockColors.input);
        this.appendDummyInput()
            .appendField("acceleration (mg)");
        this.appendDummyInput()
            .appendField(new Blockly.FieldDropdown(accelAxisDropdown), "AXIS");
        this.setInputsInline(true);
        this.setOutput(true, "Number");
        this.setTooltip('Get the acceleration on an axis (between -2048 and 2047).');
    }
};


Blockly.Blocks['device_set_onboard_led'] = {
    init: function () {
        // TODO (alecont): link below is not right
        this.setHelpUrl('https://www.microbit.co.uk/functions/digital-write-pin');
        this.setColour(blockColors.basic);
        this.appendDummyInput()
            .appendField("turn onboard LED ")
            .appendField(new Blockly.FieldDropdown(ledStateDropdown), "STATE");
        this.setInputsInline(true);
        this.setPreviousStatement(true);
        this.setNextStatement(true);
        this.setTooltip('Turn the onboard LED on or off (works only on Raspberry Pi 2, not on Raspberry Pi 3).');
    }
};

Blockly.Blocks['device_digital_write_pin'] = {
    init: function () {
        // TODO (alecont): link below is not right
        this.setHelpUrl('https://www.microbit.co.uk/functions/digital-write-pin');
        this.setColour(blockColors.pins);
        this.appendDummyInput()
            .appendField("digital write ");
        this.appendValueInput("VALUE")
            .setCheck("Number");
        this.appendDummyInput()
            .appendField(" to pin ");
        this.appendValueInput("PIN")
            .setCheck("Number");
        this.setInputsInline(true);
        this.setPreviousStatement(true);
        this.setNextStatement(true);
        this.setTooltip('Turn a specific a GPIO pin on or off.');
    }
};
