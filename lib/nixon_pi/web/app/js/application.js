jQuery(function ($) {

    Pace.Options = {
        // Disable the 'elements' source
        //elements: false,
        ajax: true,
        eventLag: true

        // Only show the progress on regular and ajax-y page navigation,
        // not every request
        //restartOnRequestAfter: false
    };

    var states = {
        'free_value': function () {
            $('#tubes').val("");
            $(".tube_animation_group").hide();
            $('.tube_time_group').hide();
            $("#countdown_examples").hide();
        },

        'animation': function () {
            $('#tubes').val("");
            $(".tube_animation_group").show();
            $('.tube_time_group').hide();
            $("#countdown_examples").hide();
        },

        'time': function () {
            $('.tube_time_group').show();
            $("#tubes").val("");
            $(".tube_animation_group").hide();
            $("#countdown_examples").hide();
        },

        'countdown': function () {
            $(".tube_animation_group").hide();
            $('.tube_time_group').hide();
            $("#countdown_examples").show();

        },

        'meeting_ticker': function () {
            $('#tubes').val("attendees:hourly_rate");
            $(".tube_animation_group").hide();
            $('.tube_time_group').hide();
            $("#countdown_examples").hide();
        }
    };

    /**
     * Display error notification
     * @param message text
     */
    var errorNotification = function (message) {
        $.pnotify({
            text: message,
            type: 'error',
            icon: 'icon-error',
            nonblock: true,
            nonblock_opacity: .2
        });
    };

    /**
     * Handle AJAX failure
     * @param err ajax error
     */
    var onFail = function (err) {
        errorNotification($.parseJSON(err.responseText).message);
    };


    var initializeColorPicker = function () {
        var $container = $('#colorpicker'),
            color_field = '#color_field';

        if ($container.length === 1) {
            $container.farbtastic(color_field);

            $(color_field).on('change', function () {
                $(this).closest("form").submit();
            });
        }
    };

    /**
     * Initialize power button
     */
    var loadPowerButtonState = function ($powerToggle) {
        $.getJSON('/information/power.json').done(function (data) {
            if (data.value == 1) {
                $powerToggle.toggleButtons('toggleState');
            }
        }).fail(onFail);
    };


    var initializePowerButton = function () {
        var $powerToggleButton = $('#power-toggle-button');


        if ($powerToggleButton.length === 1) {
            loadPowerButtonState($powerToggleButton);

            $powerToggleButton.toggleButtons({
                width: 100,
                height: 30,
                font: {
                    'font-size': '20px'
                },
                animated: true,
                transitionspeed: 0.5, // Accepted values float or "percent" [ 1, 0.5, "150%" ]
                label: {
                    enabled: "I",
                    disabled: "0"
                },
                style: {
                    enabled: "success",
                    disabled: "danger"
                },
                onChange: function ($el, status, e) {
                    $el.closest("form").submit();
                }
            });
        }
    };

    /**
     * Initialize tube data
     */
    var loadTubeState = function ($tubes) {

        $.getJSON('/information/tubes.json').done(function (data) {
            if (data.value && data.value.length > 0) {
                $tubes.val(data.value);
            }

        }).fail(onFail);

    };

    var initializeTubes = function () {
        var $tubes = $('#tubes');

        if ($tubes.length === 1) {
            loadTubeState($tubes);

            states['free_value']();

            $("#tube_state_select").chosen().change(function (event) {
                states[$(event.target).val()]();
            });
        }
    };


    /**
     * Initialize lamp data
     */
    var loadLampState = function () {
        $.getJSON('/information/lamps.json').done(function (data) {

            /**
             * Data looks like this:

             */

            var lamps = data.data;
            for (var i = 0, ii = lamps.length; i < ii; i++) {
                var state = lamps[i].state,
                    value = lamps[i].value;

                if (state == "free_value") {
                    var checked = (value == "1"),
                        $el = $('#lamp_' + i);
                    if (checked) {
                        $el.button('toggle')
                            .children("i:first")
                            .removeClass('icon-circle-blank')
                            .addClass('icon-circle')
                            .children("input[name=value]").attr("checked", checked);
                    }
                }
            }
        }).fail(onFail);

    };

    var initializeLampButtons = function () {
        var $lampButton = $(".btn-lamp");

        if ($lampButton.length > 1) {

            loadLampState();

            $lampButton.click(function () {
                var $checkBoxes = $(this).children("input[name=value]");
                var checked = !$checkBoxes.attr("checked");
                $checkBoxes.attr("checked", checked);
                if (checked) {
                    $(this).children("i:first").removeClass('icon-circle-blank').addClass('icon-circle');
                } else {
                    $(this).children("i:first").removeClass('icon-circle').addClass('icon-circle-blank');
                }
                $(this).closest("form").submit();
            });
        }
    };

    var loadBarSliderStates = function ($rangeSliders) {
        $.getJSON('/information/bars.json ').done(function (data) {
            var bars = data.bars;

            for (var i = 0, ii = bars.length; i < ii; i++) {
                var state = bars[i].state,
                    value = bars[i].value,
                    bar_num = bars[i].options.bar;

                if (state == "free_value") {
                    $rangeSliders.eq(bar_num + 1).data('rangeinput').setValue(value);
                }
            }
        }).fail(onFail);
    };

    var initializeBarSliders = function () {
        var $rangeSliders = $(':range');

        if ($rangeSliders.length > 0) {

            loadBarSliderStates($rangeSliders);

            $rangeSliders.rangeinput({
                progress: true
            });

            $(":range").change(function (event, value) {
                $(this).closest("form").submit();
            });
        }
    };

    var handleFormSubmit = function () {
        $(document).on('submit', 'form[data-remote]', function (e) {
            e.preventDefault();
            self = $(this);
            $.ajax({
                url: self.attr('action'),
                data: self.serializeArray(),
                type: self.attr('method'),
                dataType: 'json',
                success: function (res) {
                    var value = res['value'],
                        text = res['message'] + " : " + value;

                    $.pnotify({
                        text: text,
                        type: 'success',
                        icon: 'icon-success',
                        nonblock: true,
                        nonblock_opacity: .2
                    });
                },
                error: function (res, textStatus, errorMsg) {
                    var response_json = jQuery.parseJSON(res.responseText),
                        messages = response_json.message,
                        text = "";

                    if (typeof messages === 'string') {
                        messages = [messages];
                    }

                    for (i in messages) {
                        text += messages[i] + "<br/>";
                    }
                    $.pnotify({
                        text: text,
                        type: 'error',
                        icon: 'icon-error',
                        nonblock: true,
                        nonblock_opacity: .2
                    });
                }
            })
        });
    };

    var initializeAnimations = function () {
        $('select[id$=_animation]').chosen().change(function (event) {
            var animation_name = event.target.value;
            var $target = $(this).closest('form').find('textarea[id$=animation_options]').first();
            $.getJSON('information/' + animation_name.toLowerCase() + '.json').done(function (data) {
                delete data.success;
                delete data.message;
                $target.val(JSON.stringify(data[animation_name], null, 2))
            }).fail(onFail);
        });
    };


    var initializeReceivers = function ($receivers) {
        if ($receivers.length >= 1) {
            $.getJSON('receivers.json').done(function (data) {
                delete data.success;
                delete data.message;
                var targets = data.commands.receivers;

                $.each(targets, function () {
                    $receivers.append($("<option />").val(this.toString()).text(this.toString()));
                    $receivers.trigger("list:updated")
                });
            }).fail(onFail);
        }
    };

    var initializeScheduler = function () {
        if ($('#scheduler').length === 1) {
            $("#method").chosen().change(function (event) {
                var newValue = $(event.target).val(),
                    scheduler_times = {
                        'in': '1h2m3s',
                        'at': new Date().toUTCString(),
                        'every': '10s'
                    },
                    result;

                result = scheduler_times[newValue];

                if (newValue == 'cron') {
                    $('#cron').show();
                } else {
                    $('#cron').hide();
                }

                $('#time').val(result);
            });

            $("#receivers").chosen().change(function (event) {
                var target = $(event.target).val();

                $.getJSON('command/' + target.toLowerCase() + '.json').done(function (data) {
                    delete data.success;
                    delete data.message;

                    // TODO: Hu?
                    for (var commands_key in data) break;

                    var commands = data[commands_key];
                    if ('commands' in commands) {
                        commands = commands.commands;
                    }
                    var commands_hash = {};

                    $.each(commands, function (index, value) {
                        if (value == 'options') {
                            commands_hash[value] = {}
                        } else {
                            commands_hash[value] = ''
                        }
                    });

                    $('[name="command"]').val(JSON.stringify(commands_hash, null, 2))
                }).fail(onFail);
            });

            $("#cron").cron({
                onChange: function () {
                    $("#time").val($(this).cron("value"));
                }
            });

            $('#time-defaults li a').click(function () {
                $('#time').val($(this).text());
            });

            $(".delete-schedule").click(function () {
                var $this = $(this);
                var id = $this.data('id');
                $.post('/schedule/' + id, {_method: 'delete'}).done(function (result_data) {
                    $($this).closest("tr").remove();
                }).error(function () {
                    console.log("Error trying to DELETE");
                }).fail(onFail);
                return false; // prevents default behavior
            });
        }
    };

    $(document).ready(function () {

        $.pnotify.defaults.delay = 1500;
        $(".chzn-select").chosen();

        initializeColorPicker();
        initializePowerButton();
        initializeLampButtons();
        initializeBarSliders();

        initializeTubes();
        initializeAnimations();
        initializeReceivers($('#receivers'));
        initializeScheduler();
        handleFormSubmit();
    });
});