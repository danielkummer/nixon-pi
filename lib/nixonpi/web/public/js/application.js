$(document).ready(function () {

    if ($('#power-toggle-button').length === 1) {
        $.getJSON('/info/power.json', function (data) {
            if (data.value == 1) {
                $('#power-toggle-button').toggleButtons('toggleState');
            }
        });


        $('#power-toggle-button').toggleButtons({
            onChange:function ($el, status, e) {
                $el.closest("form").submit();
                console.log($el, status, e);
            },
            width:150,
            height:60,
            font:{
                'font-size':'40px'
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

    if ($('#tubes').length === 1) {
        $.getJSON('/info/tubes.json', function (data) {
            if (data.value.length > 0) {
                $('#tubes').val(data.value);
            }
        });
    }

    if ($('btn-lamp').length > 0) {
        $.getJSON('/info/lamps.json', function (data) {
            var values = data.values;
            if (!values) {
                return;
            }

            for (var i = 0, len = values.length; i < len; i++) {
                var checked = !(values[i] == 0),
                    $el = $('#lamp_' + i);

                if (checked) {
                    $el.button('toggle')
                        .children("i:first")
                        .removeClass('icon-circle-blank')
                        .addClass('icon-circle')
                        .children("input[name=values\\[\\]]").attr("checked", checked);

                }

            }
        });

        $(".btn-lamp").click(function () {
            var $checkBoxes = $(this).children("input[name=values\\[\\]]");
            var checked = !$checkBoxes.attr("checked");
            $checkBoxes.attr("checked", checked);

            console.debug("Checked:" + checked);

            if (checked) {
                $(this).children("i:first").removeClass('icon-circle-blank').addClass('icon-circle');
            } else {
                $(this).children("i:first").removeClass('icon-circle').addClass('icon-circle-blank');
            }


            $(this).closest("form").submit();
        });
    }

    if ($(':range').length > 0) {
        $.getJSON('/info/bars.json ', function (data) {
            if (data.state == "free_value") {
                //todo set rangeinput values!!
                //"values":[145,143,104,87]

                var values = data.values;

                for (var i = 0, ii = values.length; i < ii; i++) {
                    $(':range').eq(i).data('rangeinput').setValue(values[i]);
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


    //$("#form_field").trigger("liszt:updated");


    $(document).on('submit', 'form[data-remote]', function (e) {
        e.preventDefault();
        self = $(this);
        console.log("form sent with: " + self.serializeArray());
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
                var $alert = $("<div class='alert alert-error fade in'><button data-dismiss='alert' class='close' type='button'>×</button>" + errorMsg + "</div>")
                $('#alert-container').append($alert);
            }
        })
    });


    var states = {
        'free_value':function () {
            $(".tube_animation_group").hide();
            $('.tube_time_group').hide();
            $("#countdown_examples").hide();
        },

        'animation':function () {
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

        }

    };

    states['free_value']();

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
            $('[name="command"]').val(JSON.stringify(data, null, 2))
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
            console.log('DELETE result', result_data);
            $($this).closest("tr").remove();
        }).error(function () {
                console.log("Error trying to DELETE");
            });
        return false; // prevents default behavior
    });
});