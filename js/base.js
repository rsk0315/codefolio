$(window).on('load', function() {
    if (document.queryCommandSupported('copy')) {
        $('pre').each(function(i) {
            var id = 'pre-'+i;
            $(this).attr('id', id);
            $(this).before('<div class="div-btn-copy"><span class="btn-copy btn-pre" tabindex="0" data-toggle="tooltip" data-trigger="manual" title="Copied!" data-target="'+id+'">Copy</span></div>');
        });
    }

    $('.btn-copy').click(function() {
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
