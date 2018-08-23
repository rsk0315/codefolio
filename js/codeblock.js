$(window).on('load', function() {
    $('.cmd-in, .cmd-cont').prepend($('<span class="space">'));
    // $('.cmd-out').attr({
    //     'data-content': $(this),
    // });
    $.each($('.cmd-out'), function() {
        var text = $(this).text();
        $(this).attr({'data-content': text});
        $(this).text('');
    });

    $('.ps1').click(function() {
        window.getSelection().removeAllRanges();
        try {
            var range = document.createRange();
            range.selectNode($('#'+$(this).data('target')).get(0));
            window.getSelection().addRange(range);
            document.execCommand('copy');
            $(this).tooltip('show');
            var _this = this;
            setTimeout(function() { $(_this).tooltip('hide'); }, 800);
        } catch (err) {
            console.log(err);
        }
        window.getSelection().removeAllRanges();
    });

    $('[data-toggle="tooltip"]').tooltip();
});
