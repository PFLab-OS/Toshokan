(function($) {
    // 'use strict';
    var githubGimmick = {
        name: 'github',
        version: $.md.version,
        once: function() {
            $.md.linkGimmick(this, 'github', create_script);
        }
    };
    $.md.registerGimmick(githubGimmick);
    
    function create_script($links, opt, text) {
        return $links.each (function (i, link){
            var $link = $(link);
            var href = "https://gist-it.appspot.com/" + $link.attr('href');

            var $iframe = $('<iframe>');
            var iframe = $iframe.get(0);
            iframe.width = "100%";
            iframe.frameBorder = "0";
            $link.replaceWith(iframe);

            var doc = iframe.contentWindow.document;
            doc.open();
            doc.write("<" + "body style='margin:0px;padding:0px;'><div id='iframediv'><" + "script src=\"" + href + "\"><" + "/script><" + "/div><" + "/body>");
            doc.close();

            var updateSizeFn = function () {
                /*var offset = $iframe.offset();
                  var winHeight = $(window).height();
                  var newHeight = winHeight - offset.top - 5;*/
                $iframe.height($iframe.contents().find('#iframediv').height() + "px");
            };

            $iframe.load(function(done) {
                updateSizeFn();
            });
            $(window).resize(function () {
                updateSizeFn();
            });
        });
    }
}(jQuery));
(function($) {
    // 'use strict';
    var questionGimmick = {
        name: 'question',
        version: $.md.version,
        once: function() {
            $.md.linkGimmick(this, 'question', create_script);
        }
    };
    $.md.registerGimmick(questionGimmick);
    
    
    function create_script($links, opt, text) {
        return $links.each (function (i, link){
            var $link = $(link);
            var $div = $('<div>');
            var $answer = $('<p>').css({
                display: 'none',
            });
            var id = Math.floor(1000000000000*Math.random()).toString(16);
            for (var i = 0; i < opt.list.length; i++) {
                var $p = $('<p>');
                var $input = $('<input>').attr({
                    type: 'radio',
                    name: id,
                }).css({
                    "margin-right": "1em",
                }).on('click',function(my){
                    if (opt.answer != undefined && (opt.answer < opt.list.length)) {
                        if (my == opt.answer) {
                            $answer.text("正解！");
                        } else {
                            $answer.text("不正解（正解は" + (opt.answer + 1) + "）");
                        }
                    } else {
                        $answer.text("エラー：正解が設定されていません");
                    }
                    $answer.css({
                        display: 'block',
                    });
                }.bind(null, i));
                $p.append($input);
                $input.after((i + 1) + ": " + opt.list[i]);
                $div.append($p);
            }
            $div.append($answer);
            $link.replaceWith($div);
        });
    }
}(jQuery));
