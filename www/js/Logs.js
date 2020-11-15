//***************************************************************************
// Copyright 2007-2020 Universidade do Porto - Faculdade de Engenharia      *
// Laboratório de Sistemas e Tecnologia Subaquática (LSTS)                  *
//***************************************************************************
// This file is part of DUNE: Unified Navigation Environment.               *
//                                                                          *
// Commercial Licence Usage                                                 *
// Licencees holding valid commercial DUNE licences may use this file in    *
// accordance with the commercial licence agreement provided with the       *
// Software or, alternatively, in accordance with the terms contained in a  *
// written agreement between you and Faculdade de Engenharia da             *
// Universidade do Porto. For licensing terms, conditions, and further      *
// information contact lsts@fe.up.pt.                                       *
//                                                                          *
// Modified European Union Public Licence - EUPL v.1.1 Usage                *
// Alternatively, this file may be used under the terms of the Modified     *
// EUPL, Version 1.1 only (the "Licence"), appearing in the file LICENCE.md *
// included in the packaging of this file. You may not use this work        *
// except in compliance with the Licence. Unless required by applicable     *
// law or agreed to in writing, software distributed under the Licence is   *
// distributed on an "AS IS" basis, WITHOUT WARRANTIES OR CONDITIONS OF     *
// ANY KIND, either express or implied. See the Licence for the specific    *
// language governing permissions and limitations at                        *
// https://github.com/LSTS/dune/blob/master/LICENCE.md and                  *
// http://ec.europa.eu/idabc/eupl.html.                                     *
//***************************************************************************
// Author: Ricardo Martins                                                  *
//***************************************************************************

function Logs(root_id)
{
    this.create('Logs', root_id);

};

Logs.prototype = new BasicSection;

Logs.prototype.update = function()
{
    if (!g_dune_logs)
        return;

    while (this.m_base.hasChildNodes())
    {
        this.m_base.removeChild(this.m_base.lastChild);
    }

    for (var i in g_dune_logs)
    {
        var m_tbl = document.createElement('table');
        var th = document.createElement('th');
        var dirs = g_dune_logs[i].date.split("/");
        var date = dirs[dirs.length-1];
        date = date.substring(0, 4) + "-" + date.substring(4, 6) + "-" + date.substring(6, 8);
        th.appendChild(document.createTextNode(date));
        th.colSpan = 4;
        var tr1 = document.createElement('tr');
        tr1.appendChild(th);
        m_tbl.appendChild(tr1);


        for (var j in g_dune_logs[i].times)
        {
            var dirs = g_dune_logs[i].times[j].time.split("/");
            var time = dirs[dirs.length-1];
            time = time.substring(0, 2) + ":" + time.substring(2, 4) + ":" + time.substring(4, 6);

            var tr2 = document.createElement('tr');
            var td1 = document.createElement('td');
            td1.appendChild(document.createTextNode(time));
            var td2 = document.createElement('td');
            td2.appendChild(document.createTextNode(g_dune_logs[i].times[j].size));
            var td3 = document.createElement('td');

            // Delete button.
            var btn = document.createElement('input');
            btn.type = 'button';
            var curr_date
            btn.onclick = requestFileDeletion.bind(this, [i, j]);
            btn.value = 'Delete';
            td3.appendChild(btn);
            tr2.appendChild(td1);
            tr2.appendChild(td2);
            tr2.appendChild(td3);
            m_tbl.appendChild(tr2);
        }
        this.m_base.appendChild(m_tbl);
    }
};

Logs.prototype.createFolder = function(root, tree)
{
    var td = document.createElement('td');
    td.appendChild(document.createTextNode(root));

    // for (var i in tree)
    // {
    //     var e = this.createFile(tree[i].file);
    // }

    var tr = document.createElement('tr');
    tr.appendChild(td);

    return tr;
};

Logs.prototype.createFile = function(root)
{
    var li = document.createElement('li');
    li.appendChild(document.createTextNode(i));
    ul.appendChild(li);
};

function requestFileDeletion(args, event)
{
    var url = 'delete' + g_dune_logs[args[0]].times[args[1]].time.replace('"','').replace('\"', '');

    var options = Array();
    options.timeout = 10000;
    options.timeoutHandler = null;
    options.errorHandler = null;
    HTTP.get(url, handlePower, options);
}
