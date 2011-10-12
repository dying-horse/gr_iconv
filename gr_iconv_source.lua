local coroutine    = coroutine
local iconv        = iconv
local source       = source


--- <p>Unterklasse der Klasse <code>source</code> aus dem
--   Lua-Package <code>gr_source</code></p>
module "iconv.source"

--- <p>erzeugt ein Objekt der Klasse
--   <a href="/modules/iconv.source.html">iconv.source</a>.</p>
--  <p>Bevor dieses Objekt benutzt werden kann, mu&szlig; zuvor die
--   Methode
--   <a href="/modules/iconv.source.html#setencoding">
--    iconv.source.setencoding</a> aufgerufen worden sein.</p>
--  @param src ein <code>source</code>-Objekt, das einzeln umzuwandelnde
--   mb vom Typ <code>string</code> aussendet
--  @param opt (optional) table, &uuml;ber das optionale Parameter
--   &uuml;bergeben werden
--   <ul>
--    <li>Schl&uuml;ssel <code>"iconvstate"</code>: Ein Objekt der
--     der Klasse <code>iconv</code>.  Gew&ouml;hnlich wird dieser
--     Parameter mit einem Objekt einer Unterklasse von
--     <code>iconv</code> geladen, bei der die den Fehlermeldungen
--     entsprechenden Methoden <code>except_...</code> durch
--     geeignete Fehlerbehandlungsroutinen &uuml;berladen wurden.</li>
--   </ul>
--  @return besagtes Objekt
function new(self, src, opt)
 local lopt = opt or {}
 local iconvstate = lopt.iconvstate or iconv:new()
 local co = coroutine.create(
   function()
    while  (ret == "pull")
    do     coroutine.yield(iconvstate:pull())
           ret = iconvstate:go()
    end

    for inbuf in src
    do   iconvstate:push(inbuf)
         ret = iconvstate:go()

         while  (ret == "pull")
         do     coroutine.yield(iconvstate:pull())
                ret = iconvstate:go()
         end

         if     (ret == "ok")
         then   coroutine.yield(iconvstate:pull())
         end

         if     (ret == "err")
         then   break
         end
    end
   end )
 local ret = source.new(self, co)
 ret.opt   = lopt
 ret.iconvstate = iconvstate
 return ret
end

--- <p>stellt den Quell- und den Zielzeichensatz ein.</p>
--  <p>Diese Methode kann auch zwischendurch wiederholt aufgerufen,
--   wenn schon begonnen wurde, Zeichen vom Acceptor zu lesen.</p>
--  @param from Quellzeichensatz, Zeichenkette
--  @param to   Zielzeichensatz, Zeichenkette
function setencoding(self, from, to)
 self.iconvstate:open(from, to)
end
