/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 * 
 *      http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.apache.webapp.admin.connector;

import java.io.IOException;
import java.util.Collections;
import java.util.Iterator;
import java.util.Locale;
import java.util.ArrayList;
import javax.servlet.ServletException;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import org.apache.struts.action.Action;
import org.apache.struts.action.ActionForm;
import org.apache.struts.action.ActionForward;
import org.apache.struts.action.ActionMapping;

import javax.management.MBeanServer;
import javax.management.ObjectName;
import org.apache.struts.util.MessageResources;

import org.apache.webapp.admin.ApplicationServlet;
import org.apache.webapp.admin.TomcatTreeBuilder;

/**
 * The <code>Action</code> that sets up <em>Delete Connectors</em> transactions.
 *
 * @author Manveen Kaur
 * @version $Revision: 477406 $ $Date: 2006-11-20 16:34:22 -0700 (Mon, 20 Nov 2006) $
 */

public class DeleteConnectorAction extends Action {
        
    /**
     * The MBeanServer we will be interacting with.
     */
    private MBeanServer mBServer = null;
    
    
    
    
    
    // --------------------------------------------------------- Public Methods
    
    /**
     * Process the specified HTTP request, and create the corresponding HTTP
     * response (or forward to another web component that will create it).
     * Return an <code>ActionForward</code> instance describing where and how
     * control should be forwarded, or <code>null</code> if the response has
     * already been completed.
     *
     * @param mapping The ActionMapping used to select this instance
     * @param actionForm The optional ActionForm bean for this request (if any)
     * @param request The HTTP request we are processing
     * @param response The HTTP response we are creating
     *
     * @exception IOException if an input/output error occurs
     * @exception ServletException if a servlet exception occurs
     */
    public ActionForward execute(ActionMapping mapping,
    ActionForm form,
    HttpServletRequest request,
    HttpServletResponse response)
    throws IOException, ServletException {
        
        
        // Acquire the resources that we need
        Locale locale = getLocale(request);
        MessageResources resources = getResources(request);    
        
        // Acquire a reference to the MBeanServer containing our MBeans
        try {
            mBServer = ((ApplicationServlet) getServlet()).getServer();
        } catch (Throwable t) {
            throw new ServletException
            ("Cannot acquire MBeanServer reference", t);
        }
        
        String domain = null;
        // Set up a form bean containing the currently selected
        // objects to be deleted
        ConnectorsForm connectorsForm = new ConnectorsForm();
        String select = request.getParameter("select");
        if (select != null) {
            String connectors[] = new String[1];
            connectors[0] = select;
            connectorsForm.setConnectors(connectors);
                        
            // get the service Name this selected host belongs to
            try {
                domain = (new ObjectName(select)).getDomain();
            } catch (Exception e) {
                throw new ServletException
                ("Error extracting service name from the connector to be deleted", e);
            }        
        }
        request.setAttribute("connectorsForm", connectorsForm);
        
        // Accumulate a list of all available connectors
        ArrayList list = new ArrayList();
         try {
            String pattern = domain + TomcatTreeBuilder.CONNECTOR_TYPE +
                TomcatTreeBuilder.WILDCARD;          
            Iterator items =
                mBServer.queryNames(new ObjectName(pattern), null).iterator();
            while (items.hasNext()) {
                Object item = items.next();
                list.add(item.toString());
            }
        } catch (Exception e) {
            getServlet().log
                (resources.getMessage(locale, "users.error.select"));
            response.sendError
                (HttpServletResponse.SC_INTERNAL_SERVER_ERROR,
                 resources.getMessage(locale, "users.error.select"));
            return (null);
        }      
        Collections.sort(list);
        request.setAttribute("connectorsList", list);
        
        // Forward to the list display page
        return (mapping.findForward("Connectors"));        
    }
}
