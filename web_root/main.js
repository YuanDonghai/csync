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
  const [stats, setStats] = useState([]);
  const [nodes, setNodes] = useState([]); // 存储节点列表
  const [allNodes, setAllNodes] = useState([]); // 存储所有节点（用于第二个下拉框）
  const [newNodeID, setNewNodeID] = useState('');
  const [newNodeName, setNewNodeName] = useState('');
  const [selectedWssId, setSelectedWssId] = useState(''); // 新增：用于存储选择的 wss_id

  const refresh = () =>
    fetch('api/instances')
      .then(r => r.json())
      .then(r => {
        console.log('API Response:', r);
        setStats(r);
      })
      .catch(error => {
        console.error('Fetch error:', error);
      });

  // 获取节点列表
  const fetchNodes = () =>
    fetch('api/wss') // 假设这是获取节点列表的API端点
      .then(r => r.json())
      .then(r => {
        console.log('Nodes fetched:', r);
        setNodes(r);
      })
      .catch(error => {
        console.error('Error fetching nodes:', error);
      });

  // 获取所有节点（用于第二个下拉框）
  const fetchAllNodes = () =>
    fetch('api/nodes') // 获取所有节点的API端点
      .then(r => r.json())
      .then(r => {
        console.log('All nodes fetched:', r);
        setAllNodes(r);
      })
      .catch(error => {
        console.error('Error fetching all nodes:', error);
      });

  // 处理获取节点JSON并发送POST请求
  const handleGetNodeJson = (node) => {
    fetch('api/instances', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
      },
      body: JSON.stringify(node), // 发送整个节点对象
    })
      .then(r => r.json())
      .then(r => {
        console.log('Node JSON sent:', r);
        refresh(); // 刷新数据
      })
      .catch(error => {
        console.error('Error sending node JSON:', error);
      });
  };

  // 新增：获取选中节点的完整对象
  const getSelectedNode = () => {
    return nodes.find(node => node.id === selectedWssId);
  };

  useEffect(() => {
    refresh();
    fetchNodes();
    fetchAllNodes(); // 组件挂载时获取所有节点
  }, []);

  return html`
    <div class="p-2">
      <div class="p-4 sm:p-2 mx-auto grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-4">
        ${stats.filter(node => !node.id).map(node => html`
          <div class="bg-white border rounded-md shadow-lg p-4 relative">
            <h3 class="font-semibold mb-2">${node.name}</h3>
            ${node.id ? '' : html`
              <div class="mt-2">
                <input
                  type="text"
                  value=${node.name || ''}
                  onchange=${e => {
              const updatedStats = [...stats];
              const index = updatedStats.findIndex(n => n.id === node.id);
              if (index !== -1) {
                updatedStats[index] = { ...updatedStats[index], name: e.target.value };
                setStats(updatedStats);
              }
            }}
                  class="mt-1 block w-full rounded-md border-gray-300 shadow-sm focus:border-indigo-500 focus:ring-indigo-500"
                />
              </div>
              <div class="flex items-center space-x-4">
          <select
            value=${selectedWssId}
            onchange=${e => setSelectedWssId(e.target.value)}
            class="p-2 border rounded-md w-64 h-10"
          >
            <option value="" disabled selected>Select wss_id</option>
            ${nodes.map(node => html`
              <option value="${node.id}">${node.name}</option>
            `)}
          </select>

          <div class="mb-1 mt-3 flex place-content-end">
            <${Button} icon=${Icons.save} onclick=${() => handleGetNodeJson(getSelectedNode())} title="add node" />
          </div>
        </div>   
            `}
            <div class="space-y-2 mt-4">
              <div>name: ${node.name || 'N/A'}</div>
              <div>id: ${node.id || 'N/A'}</div>
              <div>wss_id: ${node.wss_id || 'N/A'}</div>        
              <div>peer_node: ${node.peer_node || 'N/A'}</div>
              <div>peer_iid: ${node.peer_iid || 'N/A'}</div>
              <div>status: ${node.status || 'N/A'}</div>
            </div>
          </div>
        `)}
      </div>
    </div>
  `;
}
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
  const getOsName = (osType) => {
    switch (osType) {
      case 0:
        return 'Windows';
      case 1:
        return 'Linux';
      default:
        return 'Unknown';
    }
  }
  return html`
<div class="p-2">
  <div class="p-4 sm:p-2 mx-auto grid grid-cols-2 lg:grid-cols-4 gap-4">
    <div class="bg-white col-span-2 border rounded-md shadow-lg p-4">
      <h3 class="font-semibold mb-2">${stats.name}</h3>
      <div class="space-y-2">
        <div>name: ${stats.name || 'namerserver'}</div>
        <div>  id: ${stats.id || 'id'}</div>
        <div>type: ${getOsName(stats.os_type)}</div>
      </div>
    </div>
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
  const [nodes, setNodes] = useState([]); // 存储节点列表
  const [allNodes, setAllNodes] = useState([]); // 存储所有节点（用于第二个下拉框）
  const [newNodeID, setNewNodeID] = useState('');
  const [newNodeName, setNewNodeName] = useState('');

  const refresh = () =>
    fetch('api/instances')
      .then(r => r.json())
      .then(r => {
        console.log('API Response:', r);
        setStats(r);
      })
      .catch(error => {
        console.error('Fetch error:', error);
      });

  // 获取节点列表
  const fetchNodes = () =>
    fetch('api/wss') // 假设这是获取节点列表的API端点
      .then(r => r.json())
      .then(r => {
        console.log('Nodes fetched:', r);
        setNodes(r);
      })
      .catch(error => {
        console.error('Error fetching nodes:', error);
      });

  // 获取所有节点（用于第二个下拉框）
  const fetchAllNodes = () =>
    fetch('api/nodes') // 获取所有节点的API端点
      .then(r => r.json())
      .then(r => {
        console.log('All nodes fetched:', r);
        setAllNodes(r);
      })
      .catch(error => {
        console.error('Error fetching all nodes:', error);
      });

  const handleEditNode = (node) => {
    // 这里处理编辑逻辑，例如打开一个模态框或导航到编辑页面
    console.log('Editing node:', node);
    // 示例：打开一个模态框
    // setSelectedNode(node);
    // setIsEditing(true);
  };

  const handleAddNode = () => {
    // 确保两个下拉框都有值
    if (!newNodeName || !newNodeID) {
      console.error('Please select both wss_id and node to share');
      return;
    }

    fetch('api/instances', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
      },
      body: JSON.stringify({
        "wss_id": newNodeName, // 使用第一个下拉框的值
        "type": 2,
        "nodes": [newNodeID] // 使用第二个下拉框的值
      }),
    })
      .then(r => r.json())
      .then(r => {
        console.log('Node added:', r);
        setNewNodeName('');
        setNewNodeID('');
        refresh();
        fetchNodes(); // 刷新节点列表
        fetchAllNodes(); // 刷新所有节点列表
      })
      .catch(error => {
        console.error('Error adding node:', error);
      });
  };

  useEffect(() => {
    refresh();
    fetchNodes();
    fetchAllNodes(); // 组件挂载时获取所有节点
  }, []);

  return html`
    <div class="p-2">
      <div class="mb-4 p-4 bg-white border rounded-md shadow-lg">
        <div class="flex items-center space-x-4">
          <select
            value=${newNodeName}
            onchange=${e => setNewNodeName(e.target.value)}
            class="p-2 border rounded-md w-64 h-10"
          >
            <option value="" disabled selected>Select wss_id</option>
            ${nodes.map(node => html`
              <option value="${node.id}">${node.name}</option>
            `)}
          </select>

          <select
            value=${newNodeID}
            onchange=${e => setNewNodeID(e.target.value)}
            class="p-2 border rounded-md"
          >
            <option value="" disabled selected>Select nodes to share</option>
            ${allNodes.map(node => html`
              <option value="${node.id}">${node.name}</option>
            `)}
          </select>

          <div class="mb-1 mt-3 flex place-content-end">
            <${Button} icon=${Icons.save} onclick=${handleAddNode} title="add node" />
          </div>
        </div>       
      </div>

      <div class="p-4 sm:p-2 mx-auto grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-4">
  ${stats.map(node => html`
    <div class="bg-white border rounded-md shadow-lg p-4 relative">
      ${node.id ? '' : html`
        <div class="absolute top-2 right-2">
          <button 
            class="bg-blue-500 hover:bg-blue-700 text-white font-bold py-1 px-2 rounded"
            onclick=${() => handleEditNode(node)}
          >
            Edit
          </button>
        </div>
        <h3 class="font-semibold mb-2">lookl</h3>
      `}
      <h3 class="font-semibold mb-2">${node.name}</h3>
      <div class="space-y-2">
        <div>name: ${node.name || 'N/A'}</div>
        <div>id: ${node.id || 'N/A'}</div>
        <div>wss_id: ${node.wss_id || 'N/A'}</div>        
        <div>peer_node: ${node.peer_node || 'N/A'}</div>
        <div>peer_iid: ${node.peer_iid || 'N/A'}</div>
        <div>status: ${node.status || 'N/A'}</div>
      </div>
    </div>
  `)}
</div>
    </div>
  `;
}

function Workspace({ }) {
  const [stats, setStats] = useState([]);
  const [newpath, setWssPath] = useState(''); // 用于存储输入框的值  
  const [newwssname, setWSSName] = useState(''); // 用于存储输入框的值

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
        "name": newwssname,
        "path": newpath
      }), // 发送新节点的名称
    })
      .then(r => r.json())
      .then(r => {
        console.log('Node added:', r); // 调试用
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
            value=${newwssname}
            onchange=${e => setWSSName(e.target.value)}
            placeholder="name"
            class="p-2 border rounded-md w-64 h-10"
          />          
        </div>
        <div class="flex items-center space-x-4">
          <input
            type="text"
            value=${newpath}
            onchange=${e => setWssPath(e.target.value)}
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
        <h3 class="font-semibold mb-2">${node.name}</h3>
        <div class="space-y-2">
          <div>name: ${node.name || 'N/A'}</div>
          <div>id: ${node.id || 'N/A'}</div>
          <div>path: ${node.path || 'N/A'}</div>        
        </div>
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
        "address": newNodeAddr,
      }), // 发送新节点的名称
    })
      .then(r => r.json())
      .then(r => {
        console.log('Node added:', r); // 调试用       
        setNewNodeAddr('');
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
            value=${newNodeAddr}
            onchange=${e => setNewNodeAddr(e.target.value)}
            placeholder="address"
            class="p-2 border rounded-md w-64 h-10"
          />          
        
        <div class="mb-1 mt-3 flex place-content-end">
          <${Button} icon=${Icons.save} onclick=${handleAddNode} title="add node" />
        </div>
      </div>

      <div class="p-4 sm:p-2 mx-auto grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-4">
        ${stats.map(node => html`
        <div class="bg-white border rounded-md shadow-lg p-4">
        <h3 class="font-semibold mb-2">${node.name}</h3>
        <div class="space-y-2">
          <div>name: ${node.name || 'N/A'}</div>
          <div>id: ${node.id || 'N/A'}</div>
          <div>address: ${node.address || 'N/A'}</div>
          <div>url: ${node.api_url || 'N/A'}</div>
          <div>port: ${node.sync_port || 'N/A'}</div>
        </div>
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
