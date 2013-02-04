(function ($) {
    $(document).ready(function () {


        $.getJSON('/state.json', function (data) {

            /*if (!data.rabbitmq) {
                var $alert = $("<div class='alert alert-error fade in'><button data-dismiss='alert' class='close' type='button'>×</button>RabbitMQ broker not running!</div>")
                $('#alert-container').append($alert);
            }*/

            if (!data.service) {
                var $alert = $("<div class='alert alert-error fade in'><i class='icon-ok-circle' />RabbitMQ up and running</div>")
                $('#state-container').append($alert);
            } else {
                $('#state-container').append($("<i class='icon-ok-circle' />RabbitMQ up and running"));

            }
        });

        /**
         * Initialize power button
         */
        if ($('#power-toggle-button').length === 1) {
            $.getJSON('/information/power.json', function (data) {
                if (data.value == 1) {
                    $('#power-toggle-button').toggleButtons('toggleState');
                }
            });

            $('#power-toggle-button').toggleButtons({
                onChange:function ($el, status, e) {
                    $el.closest("form").submit();
                },
                width:100,
                height:30,
                font:{
                    'font-size':'20px'
                },
                animated:true,
                transitionspeed:0.5, // Accepted values float or "percent" [ 1, 0.5, "150%" ]
                label:{
                    enabled:"I",
                    disabled:"0"
                },
                style:{
                    enabled:"success",
                    disabled:"danger"
                }
            });
        }


        /**
         * Initialize tube data
         */
        if ($('#tubes').length === 1) {
            $.getJSON('/information/tubes.json', function (data) {
                if (data.value.length > 0) {
                    $('#tubes').val(data.value);
                }
            });
        }

        /**
         * Initialize lamp data
         */
        if ($('#lamps').length === 1) {
            $.getJSON('/information/lamps.json', function (data) {
                var lamps = data.lamps;
                for (var i = 0, ii = lamps.length; i < ii; i++) {
                    var state = lamps[i].state,
                        value = lamps[i].value;


                    if (state == "free_value") {
                        var checked = !(value == 0),
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

            });
        }


        $(".btn-lamp").click(function () {
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


        if ($(':range').length > 0) {
            $.getJSON('/information/bars.json ', function (data) {
                var bars = data.bars;

                for (var i = 0, ii = bars.length; i < ii; i++) {
                    var state = bars[i].state,
                        value = bars[i].value,
                        bar_num = bars[i].options.bar;

                    if (state == "free_value") {
                        $(':range').eq(bar_num).data('rangeinput').setValue(value);
                    }
                }
            });


            $(":range").rangeinput({
                progress:true
            });

            $(":range").change(function (event, value) {
                $(this).closest("form").submit();
            });
        }

        $(".chzn-select").chosen();

        $(document).on('submit', 'form[data-remote]', function (e) {
            e.preventDefault();
            self = $(this);
            $.ajax({
                url:self.attr('action'),
                data:self.serializeArray(),
                type:self.attr('method'),
                dataType:'json',
                success:function (res) {
                    var value = res['value'] || res['values']
                    var text = res['message'] + " : " + value;

                    var $alert = $("<div class='alert alert-success fade in'><button data-dismiss='alert' class='close' type='button'>×</button>" + text + "</div>")
                    $('#alert-container').append($alert);
                    window.setTimeout(function () {
                        $alert.alert('close');
                    }, 2000);
                },
                error:function (res, textStatus, errorMsg) {
                    var response_json = jQuery.parseJSON(res.responseText),
                        messages = response_json.message,
                        text = "";

                    if (typeof messages === 'string') {
                        messages = [ messages ];
                    }

                    for (i in messages) {
                        text += messages[i] + "<br/>";
                    }

                    var $alert = $("<div class='alert alert-error fade in'><button data-dismiss='alert' class='close' type='button'>×</button>" + text + "</div>")
                    $('#alert-container').append($alert);
                }
            })
        });


        var states = {
            'free_value':function () {
                $('#tubes').val("");
                $(".tube_animation_group").hide();
                $('.tube_time_group').hide();
                $("#countdown_examples").hide();
            },

            'animation':function () {
                $('#tubes').val("");
                $(".tube_animation_group").show();
                $('.tube_time_group').hide();
                $("#countdown_examples").hide();
            },

            'time':function () {
                $('.tube_time_group').show();
                $("#tubes").val("");
                $(".tube_animation_group").hide();
                $("#countdown_examples").hide();
            },

            'countdown':function () {
                $(".tube_animation_group").hide();
                $('.tube_time_group').hide();
                $("#countdown_examples").show();

            },

            'meeting_ticker':function () {
                $('#tubes').val("attendees:hourly_rate")
                $(".tube_animation_group").hide();
                $('.tube_time_group').hide();
                $("#countdown_examples").hide();

            }

        };

        if ($('#tubes').length === 1) {
            states['free_value']();
        }

        //todo
        if ($('#scheduler').length === 1) {

            $.getJSON('targets.json', function (data) {
                delete data.success;
                delete data.message;
                var targets = data.targets;

                var $options = $('#target');
                $.each(targets, function () {
                    $options.append($("<option />").val(this.toString()).text(this.toString()));
                    $options.trigger("liszt:updated")
                });
            });


            $("#tube_state").chosen().change(function (event) {
                states[$(event.target).val()]();
            });

            $("#tube_animation").chosen();


            $("#method").chosen().change(function (event) {
                var newValue = $(event.target).val(),
                    scheduler_times = {
                        'in':'1h2m3s',
                        'at':new Date().toUTCString(),
                        'every':'10s'
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

            $("#target").chosen().change(function (event) {
                var target = $(event.target).val();

                $.getJSON('command/' + target.toLowerCase() + '.json', function (data) {
                    delete data.success;
                    delete data.message;
                    $('[name="command"]').val(JSON.stringify(data.commands, null, 2))
                });
            });

            $("#cron").cron({
                onChange:function () {
                    $("#time").val($(this).cron("value"));
                }
            });

            $('#time-defaults li a').click(function () {
                $('#time').val($(this).text());
            });

            $(".delete-schedule").click(function () {
                var $this = $(this);
                var id = $this.data('id');
                $.post('/schedule/' + id, {_method:'delete'},function (result_data) {
                    $($this).closest("tr").remove();
                }).error(function () {
                        console.log("Error trying to DELETE");
                    });
                return false; // prevents default behavior
            });
        }
    });
}(jQuery));