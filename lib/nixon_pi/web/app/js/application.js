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
        initPowerToggle($('#switch-power'));
        initAmbientColorPicker();
        initBackgroundSlider();
        initBarSliders();
        initLampToggles();
        initTubeValueForm();
    });

    PNotify.prototype.options.delay = 1500;

    var stack_bar_bottom = {"dir1": "up", "dir2": "right", "spacing1": 0, "spacing2": 0};

    // Returns a function, that, as long as it continues to be invoked, will not
    // be triggered. The function will be called after it stops being called for
    // N milliseconds. If `immediate` is passed, trigger the function on the
    // leading edge, instead of the trailing.
    var debounce = function (func, wait, immediate) {
        var timeout;
        return function () {
            var context = this, args = arguments;
            var later = function () {
                timeout = null;
                if (!immediate) func.apply(context, args);
            };
            var callNow = immediate && !timeout;
            clearTimeout(timeout);
            timeout = setTimeout(later, wait);
            if (callNow) func.apply(context, args);
        };
    };

    var defaultSuccessCallback = function (response) {

        console.log(response);
        var value = response.value,
            text = response.message + " : " + value;

        new PNotify({
            title: 'Done',
            text: text,
            type: 'success',
            addclass: 'stack-bar-bottom',
            cornerclass: "",
            width: "100%",
            shadow: false,
            stack: stack_bar_bottom,
            nonblock: {
                nonblock: true,
                nonblock_opacity: .2
            }
        });
    };

    var defaultErrorCallback = function (jqXHR, textStatus, errorThrown) {
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
            addclass: "stack-bar-bottom",
            cornerclass: "",
            width: "100%",
            stack: stack_bar_bottom,
            shadow: false,
            nonblock: {
                nonblock: true,
                nonblock_opacity: .2
            }
        });
    };

    var post = function (url, data) {
        $.post(url, data).done(defaultSuccessCallback).fail(defaultErrorCallback);
    };

    var get = function (url, successCallback) {
        $.getJSON(url).done(successCallback).fail(defaultErrorCallback);
    };

    /*
     * Power
     */
    var loadPowerState = function ($powerToggleButton) {
        get('/information/power.json', function (response) {
            $powerToggleButton.bootstrapSwitch('state', !!response.value, true)
        })
    };

    var initPowerToggle = function ($powerToggleButton) {
        $powerToggleButton.bootstrapSwitch({size: 'large'});
        $powerToggleButton.on('switchChange.bootstrapSwitch', debounce(function (event, state) {
            console.log('button toggled');
            post('/power', {value: state});
            //$(this).closest("form").submit();
        }, 250));
        loadPowerState($powerToggleButton);
    };

    /*
     * Ambient rgb light
     */
    var loadAmbientColorValue = function ($container) {
        get('information/rgb.json', function (response) {
            $.farbtastic($container).setColor(response.value);
        });
    };

    var initAmbientColorPicker = function () {
        var $container = $('#colorpicker');
        $container.farbtastic(debounce(function (hexColor) {
            console.log('debounce. ' + hexColor);
            post('/rgb', {value: hexColor});
        }, 250));
        loadAmbientColorValue($container);
    };

    /*
     * Tube background slider
     */
    var loadBackgroundSliderValue = function ($container) {
        get('information/background.json', function (response) {
            $container.slider('setValue', response.value);
        });
    };

    var initBackgroundSlider = function () {
        var $container = $("#background-slider");
        $container.slider({tooltip: 'always'}).on('slide', debounce(function (slideEvt) {
            console.log(slideEvt.value);
            post('/background', {value: slideEvt.value})
        }, 250));
        loadBackgroundSliderValue($container);
    };

    /*
     * Nixie bar sliders
     */
    var loadBarSliderValues = function () {
        get('information/bars.json', function (response) {
            var barsCount = response.data.length,
                idx,
                currentData,
                value;

            for (idx = 0; idx < barsCount; ++idx) {
                currentData = response.data[idx];
                value = currentData.value;
                if (currentData.hasOwnProperty('value')) {
                    console.log('has value property: ' + value);
                    $('#slider-bar' + idx).slider('setValue', parseInt(value));
                }
            }

        });
    };

    var initBarSliders = function () {
        $('input.bar').each(function () {
            var bar = $(this);
            bar.slider({tooltip: 'always'});
            bar.on('slide', debounce(function (slideEvt) {
                post('/bar', {state: 'free_value', id: $(this).data('id'), value: slideEvt.value})
            }, 250));
        });

        loadBarSliderValues();
    };

    /*
     * Nixie lamp toggles
     */
    var loadLampValues = function () {
        get('information/lamps.json', function (response) {
            var lampsCount = response.data.length,
                idx,
                currentData,
                value;

            for (idx = 0; idx < lampsCount; ++idx) {
                currentData = response.data[idx];
                value = currentData.value;
                if (currentData.hasOwnProperty('value')) {
                    $('#switch-lamp' + idx).bootstrapSwitch('state', !!value, true)
                }
            }
        });
    };

    var initLampToggles = function () {
        var $lampToggles = $('input.lamp');
        $lampToggles.bootstrapSwitch({size: 'mini'});

        $lampToggles.on('switchChange.bootstrapSwitch', debounce(function (event, state) {
            console.log('button toggled');
            post('/lamp', {state: 'free_value', id: $(this).data('id'), value: state});
        }, 250));

        loadLampValues();
    };

    var initTubeValueForm = function () {
        $('#tube-value-form').on('submit', function(event) {
            event.preventDefault();
            console.log($(this).serializeArray());
            //post('/tubes', {state: 'free_value', value: })
            /*
             return $('#new_filter').find(':input').filter(function () {
             return $.trim(this.value).length > 0
             }).serializeJSON();
             */
        })
    };


}));

