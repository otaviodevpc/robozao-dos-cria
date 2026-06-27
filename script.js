
/* ─── UPTIME CLOCK ─── */
(function(){
  const start = Date.now();
  function pad(n){ return String(n).padStart(2,'0'); }
  function tick(){
    const e = Math.floor((Date.now()-start)/1000);
    const h=Math.floor(e/3600), m=Math.floor((e%3600)/60), s=e%60;
    const el = document.getElementById('uptime');
    if(el) el.textContent = pad(h)+':'+pad(m)+':'+pad(s);
    const mt = document.getElementById('mission-time');
    if(mt) mt.textContent = pad(h)+':'+pad(m)+':'+pad(s);
  }
  setInterval(tick, 1000);
  tick();
})();

/* ─── EQ BARS ─── */
(function(){
  const wrap = document.getElementById('eq-bars');
  if(!wrap) return;
  const cols = ['#00ffe7','#00ffe7','#00b8a0','#39ff14','#00ffe7','#39ff14','#00b8a0','#ffaa00','#00ffe7','#00b8a0','#39ff14','#00ffe7','#00b8a0','#00ffe7','#00ffe7','#ffaa00'];
  const bars = [];
  for(let i=0;i<16;i++){
    const b = document.createElement('div');
    b.style.cssText = `flex:1;background:${cols[i]};min-height:2px;transition:height 0.12s ease;`;
    wrap.appendChild(b);
    bars.push(b);
  }
  function animate(){
    bars.forEach(b=>{
      const h = Math.floor(Math.random()*36)+4;
      b.style.height = h+'px';
    });
  }
  setInterval(animate, 120);
  animate();
})();

/* ─── MISSION LOG FEED ─── */
(function(){
  const el = document.getElementById('log-output');
  if(!el) return;
  const msgs = [
    '&gt; SCANNING SECTOR...',
    '&gt; NO CHANGE DETECTED.',
    '&gt; PING SAT-LINK... OK',
    '&gt; UPDATING COORDS...',
    '&gt; TGT-001 CONFIRMED.',
    '&gt; FUEL: 87% NOMINAL.',
    '&gt; ECM STANDBY MODE.',
    '&gt; COMMS CHECK... OK',
    '&gt; AWAITING AUTH CODE.',
    '&gt; ORBIT MAINTAINED.',
  ];
  let i = 0;
  const lines = ['&gt; UNIT DEPLOYED...','&gt; AREA SCAN INIT...','&gt; TARGET LOCKED...','&gt; AWAITING CMD...'];
  setInterval(()=>{
    if(lines.length > 6) lines.shift();
    lines.push(msgs[i % msgs.length]);
    el.innerHTML = lines.join('<br>');
    i++;
  }, 2200);
})();

/* ─── COUNTER ANIMATION ─── */
(function(){
  const counters = document.querySelectorAll('.counter[data-target]');
  const io = new IntersectionObserver(entries => {
    entries.forEach(e => {
      if(!e.isIntersecting) return;
      const el = e.target;
      const target = parseInt(el.getAttribute('data-target'));
      let current = 0;
      const step = Math.max(1, Math.floor(target / 60));
      const interval = setInterval(()=>{
        current = Math.min(current + step, target);
        el.textContent = current.toLocaleString();
        if(current >= target) clearInterval(interval);
      }, 24);
      io.unobserve(el);
    });
  }, { threshold: 0.3 });})