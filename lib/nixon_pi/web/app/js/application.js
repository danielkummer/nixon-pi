/**
 * This is a rather quick and dirty javascript file to control the web ui.
 * You should write your own client to directly interact with the api instead of using this...
 */

// IIFE - Immediately Invoked Function Expression
(function (nixiecode) {
    // The global jQuery object is passed as a parameter
    nixiecode(window.jQuery, window, document);

}(function ($, window, document) {
    $(function () {
        // The DOM is ready!
        loadPowerToggle($('#switch-power'));
        initializeColorPicker();
        loadBackgroundSlider();
        loadBarSliders();
        loadLampToggles();
    });

    var defaultSuccessCallback = function (response) {

        console.log(response);
        var value = response.value,
            text = response.message + " : " + value;

        new PNotify({
            title: 'Success',
            text: text,
            type: 'success',
            nonblock: {
                nonblock: true,
                nonblock_opacity: .2
            }
        });
    };

    var defaultErrorCallback = function (jqXHR, textStatus, errorThrown) {

        console.log(textStatus);
        console.log(errorThrown);

        var response_json = jQuery.parseJSON(jqXHR.responseText),
            messages = response_json.message,
            text = "";


        if (typeof messages === 'string') {
            messages = [messages];
        }

        var idx;
        for (idx in messages) {
            text += messages[idx] + "<br/>";
        }

        new PNotify({
            title: 'Oops',
            type: 'error',
            text: text,
            nonblock: {
                nonblock: true,
                nonblock_opacity: .2
            }
        });
    };

    var post = function (url, data) {
        $.post(data).done(defaultSuccessCallback).fail(defaultErrorCallback);
    };

    var get = function (url, successCallback) {
        $.getJSON(url).done(successCallback).fail(defaultErrorCallback);
    };


    /*
     * Power
     */

    var loadPowerState = function ($powerToggleButton) {
        get('/information/power.json', function (response) {
            $powerToggleButton.bootstrapSwitch('power', response.value)
        })
    };

    var loadPowerToggle = function ($powerToggleButton) {

        $powerToggleButton.bootstrapSwitch({size: 'large'});


        $powerToggleButton.on('switchChange.bootstrapSwitch', function (event, state) {
            console.log('button toggled');
            setPower(state);
            //$(this).closest("form").submit();
        });

        loadPowerState($powerToggleButton);
    };

    var setPower = function (value) {
        //todo set toggle to false on fail
        post('/power', {value: value});
    };

    /*
     * Tubes
     */


    /*
     * Background RGB
     */

    var initializeColorPicker = function () {
        var $container = $('#colorpicker'),
            color_field = '#color_field';

        if ($container.length === 1) {
            $container.farbtastic(color_field);

            /*$(color_field).on('change', function () {
             $(this).closest("form").submit();
             });*/
        }
    };


    var loadBackgroundSlider = function () {
        var backgroundSlider = $("#background-slider").slider();
        backgroundSlider.on("slide", function (slideEvt) {
            console.log(slideEvt.value);
        });

    };

    var loadBarSliders = function () {
        $("input.bar").slider().on("slide", function (slideEvt) {
            console.log(slideEvt.value);
        });
    };

    var loadLampToggles = function () {
        $('input.lamp').bootstrapSwitch({size: 'mini'});
    };

}));

