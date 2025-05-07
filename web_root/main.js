//  NOTE: API calls must start with 'api/' in order to serve the app at any URI

'use strict';
import { h, render, useState, useEffect, useRef, html, Router } from './bundle.js';
import { Icons, Login, Setting, Button, Stat, tipColors, Colored, Notification, Pagination, UploadFileButton } from './components.js';

const Logo = props => html`<svg class=${props.class} xmlns="http://www.w3.org/2000/svg" viewBox="0 0 12.87 12.85"><defs><style>.ll-cls-1{fill:none;stroke:#000;stroke-miterlimit:10;stroke-width:0.5px;}</style></defs><g id="Layer_2" data-name="Layer 2"><g id="Layer_1-2" data-name="Layer 1"><path class="ll-cls-1" d="M12.62,1.82V8.91A1.58,1.58,0,0,1,11,10.48H4a1.44,1.44,0,0,1-1-.37A.69.69,0,0,1,2.84,10l-.1-.12a.81.81,0,0,1-.15-.48V5.57a.87.87,0,0,1,.86-.86H4.73V7.28a.86.86,0,0,0,.86.85H9.42a.85.85,0,0,0,.85-.85V3.45A.86.86,0,0,0,10.13,3,.76.76,0,0,0,10,2.84a.29.29,0,0,0-.12-.1,1.49,1.49,0,0,0-1-.37H2.39V1.82A1.57,1.57,0,0,1,4,.25H11A1.57,1.57,0,0,1,12.62,1.82Z"/><path class="ll-cls-1" d="M10.48,10.48V11A1.58,1.58,0,0,1,8.9,12.6H1.82A1.57,1.57,0,0,1,.25,11V3.94A1.57,1.57,0,0,1,1.82,2.37H8.9a1.49,1.49,0,0,1,1,.37l.12.1a.76.76,0,0,1,.11.14.86.86,0,0,1,.14.47V7.28a.85.85,0,0,1-.85.85H8.13V5.57a.86.86,0,0,0-.85-.86H3.45a.87.87,0,0,0-.86.86V9.4a.81.81,0,0,0,.15.48l.1.12a.69.69,0,0,0,.13.11,1.44,1.44,0,0,0,1,.37Z"/></g></g></svg>`;

function Header({ logout, user, setShowSidebar, showSidebar }) {
  return html`
<div class="bg-white sticky top-0 z-[48] xw-full border-b py-2 ${showSidebar && 'pl-72'} transition-all duration-300 transform">
  <div class="px-2 w-full py-0 my-0 flex items-center">
    <button type="button" onclick=${ev => setShowSidebar(v => !v)} class="text-slate-400">
      <${Icons.bars3} class="h-6" />
    <//>
    <div class="flex flex-1 gap-x-4 self-stretch lg:gap-x-6">
      <div class="relative flex flex-1"><//>
      <div class="flex items-center gap-x-4 lg:gap-x-6">
        <span class="text-sm text-slate-400">logged in as: ${user}<//>
        <div class="hidden lg:block lg:h-4 lg:w-px lg:bg-gray-200" aria-hidden="true"><//>
        <${Button} title="Logout" icon=${Icons.logout} onclick=${logout} />
      <//>
    <//>
  <//>
<//>`;
};

function Sidebar({ url, show }) {
  const NavLink = ({ title, icon, href, url }) => html`
  <div>
    <a href="#${href}" class="${href == url ? 'bg-slate-50 text-blue-600 group' : 'text-gray-700 hover:text-blue-600 hover:bg-gray-50 group'} flex gap-x-3 rounded-md p-2 text-sm leading-6 font-semibold">
      <${icon} class="w-6 h-6"/>
      ${title}
    <///>
  <//>`;
  return html`
<div class="bg-violet-100 hs-overlay hs-overlay-open:translate-x-0
            -translate-x-full transition-all duration-300 transform
            fixed top-0 left-0 bottom-0 z-[60] w-72 bg-white border-r
            border-gray-200 overflow-y-auto scrollbar-y
            ${show && 'translate-x-0'} right-auto bottom-0">
  <div class="flex flex-col m-4 gap-y-6">
    <div class="flex h-10 shrink-0 items-center gap-x-4 font-bold text-xl text-slate-500">
      <${Logo} class="h-full"/> csync
    <//>
    <div class="flex flex-1 flex-col">
      <${NavLink} title="Dashboard" icon=${Icons.home} href="/" url=${url} />
      <${NavLink} title="Nodes" icon=${Icons.cog} href="/nodes" url=${url} />
      <${NavLink} title="WorkSpace" icon=${Icons.cog} href="/wss" url=${url} />
      <${NavLink} title="Instance" icon=${Icons.cog} href="/instance" url=${url} />
      <${NavLink} title="Events" icon=${Icons.alert} href="/events" url=${url} />
    <//>
  <//>
<//>`;
};

function Events({ }) {
  const [events, setEvents] = useState([]);
  const [page, setPage] = useState(1);

  const refresh = () =>
    fetch('api/events/get', {
      method: 'POST', body: JSON.stringify({ page: page }),
    }).then(r => r.json())
      .then(r => setEvents(r));

  useEffect(refresh, [page]);

  useEffect(() => {
    setPage(JSON.parse(localStorage.getItem('page')));
  }, []);

  useEffect(() => {
    localStorage.setItem('page', page.toString());
  }, [page]);

  const Th = props => html`<th scope="col" class="sticky top-0 z-10 border-b border-slate-300 bg-white bg-opacity-75 py-1.5 px-4 text-left text-sm font-semibold text-slate-900 backdrop-blur backdrop-filter">${props.title}</th>`;
  const Td = props => html`<td class="whitespace-nowrap border-b border-slate-200 py-2 px-4 pr-3 text-sm text-slate-900">${props.text}</td>`;
  const Prio = ({ prio }) => {
    const text = ['high', 'medium', 'low'][prio];
    const colors = [tipColors.red, tipColors.yellow, tipColors.green][prio];
    return html`<${Colored} colors=${colors} text=${text} />`;
  };

  const Event = ({ e }) => html`
<tr>
  <${Td} text=${['power', 'hardware', 'tier3', 'tier4'][e.type]} />
  <${Td} text=${html`<${Prio} prio=${e.prio}/>`} />
  <${Td} text=${e.time ? (new Date(e.time * 1000)).toLocaleString() : '1970-01-01'} />
  <${Td} text=${e.text} />
<//>`;

  return html`
<div class="m-4 divide-y divide-gray-200 overflow-auto rounded bg-white">
  <div class="font-semibold flex items-center text-gray-600 px-3 justify-between whitespace-nowrap">
    <div class="font-semibold flex items-center text-gray-600">
      <div class="mr-4">EVENT LOG</div>
    </div>
    <${Pagination} currentPage=${page} setPageFn=${setPage} totalItems=400 itemsPerPage=20 />
  <//>
  <div class="inline-block min-w-full align-middle" style="max-height: 82vh; overflow: auto;">
    <table class="min-w-full border-separate border-spacing-0">
      <thead>
        <tr>
          <${Th} title="Type" />
          <${Th} title="Prio" />
          <${Th} title="Time" />
          <${Th} title="Description" />
        </tr>
      </thead>
      <tbody>
        ${(events.arr ? events.arr : []).map(e => h(Event, { e }))}
      </tbody>
    </table>
  <//>
<//>`;
};

function Chart({ data }) {
  const n = data.length /* entries */, w = 20 /* entry width */, ls = 15/* left space */;
  const h = 100 /* graph height */, yticks = 5 /* Y axis ticks */, bs = 10 /* bottom space */;
  const ymax = 25;
  const yt = i => (h - bs) / yticks * (i + 1);
  const bh = p => (h - bs) * p / 100; // Bar height
  const by = p => (h - bs) - bh(p);
  const range = (start, size, step) => Array.from({ length: size }, (_, i) => i * (step || 1) + start);
  // console.log(ds);
  return html`
<div class="my-4 divide-y divide-gray-200 overflow-auto rounded bg-white">
  <div class="font-light uppercase flex items-center text-gray-600 px-4 py-2">
  Temperature, last 24h
  <//>
  <div class="relative">
    <svg class="bg-yellow-x50 w-full p-4" viewBox="0 0 ${n * w + ls} ${h}">
      ${range(0, yticks).map(i => html`
        <line x1=0 y1=${yt(i)} x2=${ls + n * w} y2=${yt(i)} stroke-width=0.3 class="stroke-slate-300" stroke-dasharray="1,1" />
        <text x=0 y=${yt(i) - 2} class="text-[6px] fill-slate-400">${ymax - ymax / yticks * (i + 1)}<//>
      `)}
      ${range(0, n).map(x => html`
        <rect x=${ls + x * w} y=${by(data[x] * 100 / ymax)} width=12 height=${bh(data[x] * 100 / ymax)} rx=2 class="fill-cyan-500" />
        <text x=${ls + x * w} y=100 class="text-[6px] fill-slate-400">${x * 2}:00<//>
      `)}
    <//>
  <//>
<//>`;
};

function DeveloperNote({ text, children }) {
  return html`
<div class="flex p-4 gap-2">
  <div class="text-sm text-slate-500">
    <div class="flex items-center">
      <${Icons.info} class="self-start basis-[30px] grow-0 shrink-0 text-green-600 mr-2" />
      <div class="font-semibold">Developer Note<//>
    <//>
    ${(text || '').split('.').map(v => html` <p class="my-2 ">${v}<//>`)}
    ${children}
  <//>
<//>`;
};

function Main({ }) {
  const [stats, setStats] = useState(null);
  const refresh = () => fetch('api/node').then(r => r.json()).then(r => setStats(r));
  useEffect(refresh, []);
  if (!stats) return '';
  return html`
<div class="p-2">
  <div class="p-4 sm:p-2 mx-auto grid grid-cols-2 lg:grid-cols-4 gap-4">
    <${Stat} title="name" text="${stats.name}" tipText="good" tipIcon=${Icons.ok} tipColors=${tipColors.green} />  
    <${Stat} title="id" text="${stats.id}" tipText="good" tipIcon=${Icons.ok} tipColors=${tipColors.green} /> 
    <div class="bg-white col-span-2 border rounded-md shadow-lg" role="alert">
      <${DeveloperNote} text="Stats data is received from the Mongoose backend" />
    <//>
  <//>
<//>`;
};

function FirmwareStatus({ title, info, children }) {
  const state = ['UNAVAILABLE', 'FIRST_BOOT', 'NOT_COMMITTED', 'COMMITTED'][(info.status || 0) % 4];
  const valid = info.status > 0;
  return html`
<div class="bg-white py-1 divide-y border rounded">
  <div class="font-light uppercase flex items-center text-gray-600 px-4 py-2">
    ${title}
  <//>
  <div class="px-4 py-2 relative">
    <div class="my-1">Status: ${state}<//>
    <div class="my-1">CRC32: ${valid ? info.crc32.toString(16) : 'n/a'}<//>
    <div class="my-1">Size: ${valid ? info.size : 'n/a'}<//>
    <div class="my-1">Flashed at: ${valid ? new Date(info.timestamp * 1000).toLocaleString() : 'n/a'}<//>
    ${children}
  <//>
<//>`;
};


function Instance({ }) {
  const [stats, setStats] = useState([]);
  const [newNodeAddr, setNewNodeAddr] = useState(''); // 用于存储输入框的值
  const [newNodeID, setNewNodeID] = useState(''); // 用于存储输入框的值
  const [newNodeName, setNewNodeName] = useState(''); // 用于存储输入框的值
  const [newNodeAPI, setNewNodeAPI] = useState(''); // 用于存储输入框的值
  const [newNodePort, setNewNodePort] = useState(''); // 用于存储输入框的值

  const refresh = () =>
    fetch('api/instances')
      .then(r => r.json())
      .then(r => {
        console.log('API Response:', r); // 调试用
        setStats(r);
      })
      .catch(error => {
        console.error('Fetch error:', error); // 调试用
      });

  const handleAddNode = () => {
    fetch('api/wss', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
      },
      body: JSON.stringify({
        "id": newNodeID,
        "name": newNodeName,
        "path": newNodeAddr
      }), // 发送新节点的名称
    })
      .then(r => r.json())
      .then(r => {
        console.log('Node added:', r); // 调试用
        setNewNodeName(''); // 清空输入框
        setNewNodeID('');
        setNewNodeAddr('');
        setNewNodeAPI('');
        setNewNodePort('');
        refresh(); // 刷新节点列表
      })
      .catch(error => {
        console.error('Error adding node:', error); // 调试用
      });
  };

  useEffect(() => {
    refresh();
  }, []);

  return html`
    <div class="p-2">
      <div class="mb-4 p-4 bg-white border rounded-md shadow-lg">
        <div class="flex items-center space-x-4">
          <input
            type="text"
            value=${newNodeName}
            onchange=${e => setNewNodeName(e.target.value)}
            placeholder="name"
            class="p-2 border rounded-md w-64 h-10"
          />
          <input
            type="text"
            value=${newNodeID}
            onchange=${e => setNewNodeID(e.target.value)}
            placeholder="id"
            class="p-2 border rounded-md"
          />
        </div>
        <div class="flex items-center space-x-4">
          <input
            type="text"
            value=${newNodeAddr}
            onchange=${e => setNewNodeAddr(e.target.value)}
            placeholder="path"
            class="p-2 border rounded-md w-64 h-10"
          />
        

        <div class="mb-1 mt-3 flex place-content-end">
          <${Button} icon=${Icons.save} onclick=${handleAddNode} title="add node" />
        </div>
      </div>

      <div class="p-4 sm:p-2 mx-auto grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-4">
        ${stats.map(node => html`
          <div class="bg-white border rounded-md shadow-lg p-4">
            <${Stat} 
              title="Name" 
              text="${node.name}" 
            />
            <${Stat} 
              title="ID" 
              text="${node.id}" 
            />
            <${Stat} 
              title="wss id" 
              text="${node.wss_id}" 
            />    
             <${Stat} 
              title="peer" 
              text="${node.peer_node}" 
            />    
             <${Stat} 
              title="peer id" 
              text="${node.peer_iid}" 
            />    
             <${Stat} 
              title="type" 
              text="${node.type}" 
            />           
          </div>
        `)}
      </div>
    </div>
  `;
}

function Workspace({ }) {
  const [stats, setStats] = useState([]);
  const [newNodeAddr, setNewNodeAddr] = useState(''); // 用于存储输入框的值
  const [newNodeID, setNewNodeID] = useState(''); // 用于存储输入框的值
  const [newNodeName, setNewNodeName] = useState(''); // 用于存储输入框的值
  const [newNodeAPI, setNewNodeAPI] = useState(''); // 用于存储输入框的值
  const [newNodePort, setNewNodePort] = useState(''); // 用于存储输入框的值

  const refresh = () =>
    fetch('api/wss')
      .then(r => r.json())
      .then(r => {
        console.log('API Response:', r); // 调试用
        setStats(r);
      })
      .catch(error => {
        console.error('Fetch error:', error); // 调试用
      });

  const handleAddNode = () => {
    fetch('api/wss', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
      },
      body: JSON.stringify({
        "id": newNodeID,
        "name": newNodeName,
        "path": newNodeAddr
      }), // 发送新节点的名称
    })
      .then(r => r.json())
      .then(r => {
        console.log('Node added:', r); // 调试用
        setNewNodeName(''); // 清空输入框
        setNewNodeID('');
        setNewNodeAddr('');
        setNewNodeAPI('');
        setNewNodePort('');
        refresh(); // 刷新节点列表
      })
      .catch(error => {
        console.error('Error adding node:', error); // 调试用
      });
  };

  useEffect(() => {
    refresh();
  }, []);

  return html`
    <div class="p-2">
      <div class="mb-4 p-4 bg-white border rounded-md shadow-lg">
        <div class="flex items-center space-x-4">
          <input
            type="text"
            value=${newNodeName}
            onchange=${e => setNewNodeName(e.target.value)}
            placeholder="name"
            class="p-2 border rounded-md w-64 h-10"
          />
          <input
            type="text"
            value=${newNodeID}
            onchange=${e => setNewNodeID(e.target.value)}
            placeholder="id"
            class="p-2 border rounded-md"
          />
        </div>
        <div class="flex items-center space-x-4">
          <input
            type="text"
            value=${newNodeAddr}
            onchange=${e => setNewNodeAddr(e.target.value)}
            placeholder="path"
            class="p-2 border rounded-md w-64 h-10"
          />
        

        <div class="mb-1 mt-3 flex place-content-end">
          <${Button} icon=${Icons.save} onclick=${handleAddNode} title="add node" />
        </div>
      </div>

      <div class="p-4 sm:p-2 mx-auto grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-4">
        ${stats.map(node => html`
          <div class="bg-white border rounded-md shadow-lg p-4">
            <${Stat} 
              title="Name" 
              text="${node.name}" 
            />
            <${Stat} 
              title="ID" 
              text="${node.id}" 
            />
            <${Stat} 
              title="Path" 
              text="${node.path}" 
            />           
          </div>
        `)}
      </div>
    </div>
  `;
}

function Nodes({ }) {
  const [stats, setStats] = useState([]);
  const [newNodeAddr, setNewNodeAddr] = useState(''); // 用于存储输入框的值
  const [newNodeID, setNewNodeID] = useState(''); // 用于存储输入框的值
  const [newNodeName, setNewNodeName] = useState(''); // 用于存储输入框的值
  const [newNodeAPI, setNewNodeAPI] = useState(''); // 用于存储输入框的值
  const [newNodePort, setNewNodePort] = useState(''); // 用于存储输入框的值

  const refresh = () =>
    fetch('api/nodes')
      .then(r => r.json())
      .then(r => {
        console.log('API Response:', r); // 调试用
        setStats(r);
      })
      .catch(error => {
        console.error('Fetch error:', error); // 调试用
      });

  const handleAddNode = () => {
    fetch('api/nodes', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
      },
      body: JSON.stringify({
        "id": newNodeID,
        "name": newNodeName,
        "address": newNodeAddr,
        "api_url": newNodeAPI,
        "sync_port": newNodePort
      }), // 发送新节点的名称
    })
      .then(r => r.json())
      .then(r => {
        console.log('Node added:', r); // 调试用
        setNewNodeName(''); // 清空输入框
        setNewNodeID('');
        setNewNodeAddr('');
        setNewNodeAPI('');
        setNewNodePort('');
        refresh(); // 刷新节点列表
      })
      .catch(error => {
        console.error('Error adding node:', error); // 调试用
      });
  };

  useEffect(() => {
    refresh();
  }, []);

  return html`
    <div class="p-2">
      <div class="mb-4 p-4 bg-white border rounded-md shadow-lg">
        <div class="flex items-center space-x-4">
          <input
            type="text"
            value=${newNodeName}
            onchange=${e => setNewNodeName(e.target.value)}
            placeholder="name"
            class="p-2 border rounded-md w-64 h-10"
          />
          <input
            type="text"
            value=${newNodeID}
            onchange=${e => setNewNodeID(e.target.value)}
            placeholder="id"
            class="p-2 border rounded-md"
          />
        </div>
        <div class="flex items-center space-x-4">
          <input
            type="text"
            value=${newNodeAddr}
            onchange=${e => setNewNodeAddr(e.target.value)}
            placeholder="address"
            class="p-2 border rounded-md w-64 h-10"
          />
          <input
            type="text"
            value=${newNodeAPI}
            onchange=${e => setNewNodeAPI(e.target.value)}
            placeholder="API URL"
            class="p-2 border rounded-md"
          />
        </div>
        <div class="flex items-center space-x-4">
          <input
            type="number"
            value=${newNodePort}
            onchange=${e => setNewNodePort(e.target.value)}
            placeholder="Port"
            class="p-2 border rounded-md w-64 h-10"
          />           
        </div>

        <div class="mb-1 mt-3 flex place-content-end">
          <${Button} icon=${Icons.save} onclick=${handleAddNode} title="add node" />
        </div>
      </div>

      <div class="p-4 sm:p-2 mx-auto grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-4">
        ${stats.map(node => html`
          <div class="bg-white border rounded-md shadow-lg p-4">
            <${Stat} 
              title="Name" 
              text="${node.name}" 
              tipText="good" 
              tipIcon=${Icons.ok} 
              tipColors=${tipColors.green} 
            />
            <${Stat} 
              title="ID" 
              text="${node.id}" 
            />
            <${Stat} 
              title="Address" 
              text="${node.address}" 
            />
            <${Stat} 
              title="API URL" 
              text="${node.api_url}" 
            />
            <${Stat} 
              title="Sync Port" 
              text="${node.sync_port}" 
            />
          </div>
        `)}
      </div>
    </div>
  `;
}

const App = function ({ }) {
  const [loading, setLoading] = useState(true);
  const [url, setUrl] = useState('/');
  const [user, setUser] = useState('');
  const [showSidebar, setShowSidebar] = useState(true);

  const logout = () => fetch('api/logout').then(r => setUser(''));
  const login = r => !r.ok ? setLoading(false) && setUser(null) : r.json()
    .then(r => setUser(r.user))
    .finally(r => setLoading(false));

  useEffect(() => fetch('api/login').then(login), []);

  if (loading) return '';  // Show blank page on initial load
  if (!user) return html`<${Login} loginFn=${login} logoIcon=${Logo}
    title="csync management login" 
    tipText="To login, use: admin/admin, user1/user1, user2/user2" />`; // If not logged in, show login screen

  return html`
<div class="min-h-screen bg-slate-100">
  <${Sidebar} url=${url} show=${showSidebar} />
  <${Header} logout=${logout} user=${user} showSidebar=${showSidebar} setShowSidebar=${setShowSidebar} />
  <div class="${showSidebar && 'pl-72'} transition-all duration-300 transform">
    <${Router} onChange=${ev => setUrl(ev.url)} history=${History.createHashHistory()} >
      <${Main} default=${true} />
      <${Nodes} path="nodes" />
      <${Workspace} path="wss" />
      <${Instance} path="instance" />
      <${Events} path="events" />
    <//>
  <//>
<//>`;
};

window.onload = () => render(h(App), document.body);
