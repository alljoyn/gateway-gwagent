/******************************************************************************
 * Copyright (c) 2014, AllSeen Alliance. All rights reserved.
 *
 *    Permission to use, copy, modify, and/or distribute this software for any
 *    purpose with or without fee is hereby granted, provided that the above
 *    copyright notice and this permission notice appear in all copies.
 *
 *    THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 *    WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 *    MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 *    ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 *    WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 *    ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 *    OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 ******************************************************************************/

package org.alljoyn.gatewaycontroller.activity;

import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.List;

import org.alljoyn.gatewaycontroller.CallbackMethod;
import org.alljoyn.gatewaycontroller.R;
import org.alljoyn.gatewaycontroller.adapters.ConnectorAppAclsAdapter;
import org.alljoyn.gatewaycontroller.adapters.ConnectorAppsAdapter;
import org.alljoyn.gatewaycontroller.adapters.VisualAcl;
import org.alljoyn.gatewaycontroller.adapters.VisualItem;
import org.alljoyn.gatewaycontroller.sdk.Acl;
import org.alljoyn.gatewaycontroller.sdk.ConnectorApp;
import org.alljoyn.gatewaycontroller.sdk.ConnectorAppStatus;
import org.alljoyn.gatewaycontroller.sdk.ConnectorAppStatus.RestartStatus;
import org.alljoyn.gatewaycontroller.sdk.ConnectorAppStatusSignalHandler;
import org.alljoyn.gatewaycontroller.sdk.GatewayControllerException;

import android.content.DialogInterface;
import android.content.Intent;
import android.os.AsyncTask;
import android.os.Bundle;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.CompoundButton;
import android.widget.ListView;
import android.widget.TextView;

/**
 * The activity presents {@link ConnectorApp}s and its
 * {@link Acl} objects
 */
public class ConnectorAppActivity extends BaseActivity implements ConnectorAppStatusSignalHandler, OnItemClickListener {

    private static final String TAG = "gwcapp" + ConnectorAppActivity.class.getSimpleName();

    /**
     * Gateway name
     */
    private TextView gwNameTv;

    /**
     * Selected application name
     */
    private TextView connAppName;

    /**
     * Selected application version
     */
    private TextView connAppVer;

    /**
     * Application connection status
     */
    private TextView connStatus;

    /**
     * Application operational status
     */
    private TextView operStatus;

    /**
     * Application install status
     */
    private TextView installStatus;

    /**
     * The {@link ListView} of the Application ACLs
     */
    private ListView aclsListView;

    /**
     * Adapter for the list of the ACLs
     */
    private ConnectorAppAclsAdapter adapter;

    /**
     * Asynchronous task to be executed
     */
    private AsyncTask<Void, Void, Void> asyncTask;

    /**
     * Invoke this method when onSessionJoined event is called
     */
    private CallbackMethod invokeOnSessionReady;

    /**
     * Reflection of the retrieveData method
     */
    private static Method retrieveDataMethod;

    /**
     * Reflection of the restartApp method
     */
    private static Method restartAppMethod;

    /**
     * Reflection of the changeAclActiveStatus method
     */
    private static Method changeAclActiveStatusMethod;

    static {

        try {

            Class<ConnectorAppActivity> activClass = ConnectorAppActivity.class;
            retrieveDataMethod          = activClass.getDeclaredMethod("retrieveData");
            restartAppMethod            = activClass.getDeclaredMethod("restartApp");
            changeAclActiveStatusMethod = activClass.getDeclaredMethod("changeAclActiveStatus", VisualAcl.class, CompoundButton.class, boolean.class);

        } catch (NoSuchMethodException nsme) {
            Log.wtf(TAG, "NoSuchMethodException", nsme);
        }
    }

    /**
     * @see android.app.Activity#onCreate(android.os.Bundle)
     */
    @Override
    protected void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_connector_application);

        if (retrieveDataMethod == null || restartAppMethod == null || changeAclActiveStatusMethod == null) {

            Log.e(TAG, "Reflection of the required methods is undefined, can't continue working");
            showOkDialog("Error", "Internal application error./n Can't continue working.", "Ok", new DialogInterface.OnClickListener() {
                @Override
                public void onClick(DialogInterface dialog, int which) {
                    finish();
                }
            });
        }
    }

    /**
     * @see android.app.Activity#onCreateOptionsMenu(android.view.Menu)
     */
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {

        getMenuInflater().inflate(R.menu.connector_application, menu);
        return true;
    }

    /**
     * @see android.app.Activity#onStart()
     */
    @Override
    protected void onStart() {

        super.onStart();

        // Check existence of the selected gateway management app
        if (app.getSelectedGatewayApp() == null) {

            Log.w(TAG, "Selected gateway app has been lost, handling");
            handleLostOfGateway();
            return;
        }

        try {
            app.getSelectedConnectorApp().setStatusSignalHandler(this);
        } catch (GatewayControllerException gce) {

            Log.e(TAG, "Failed to setStatusChangedHandler", gce);
            showOkDialog("Error", "Failed to register Status Change Handler.", "Ok", null);
        }

        gwNameTv      = (TextView) findViewById(R.id.gwNameTv);
        gwNameTv.setText(app.getSelectedGatewayApp().getAppName());

        connAppName   = (TextView) findViewById(R.id.aclMgmtConnAppNameTv);
        connAppName.setText(app.getSelectedConnectorApp().getFriendlyName());

        connAppVer    = (TextView) findViewById(R.id.connectorAppVerTv);
        connAppVer.setText(app.getSelectedConnectorApp().getAppVersion());

        connStatus    = (TextView) findViewById(R.id.connectorAppConnStatus);
        operStatus    = (TextView) findViewById(R.id.connectorAppOperStatus);
        installStatus = (TextView) findViewById(R.id.connectorAppInstallStatus);

        adapter = new ConnectorAppAclsAdapter(this, R.layout.connector_app_acl_item, new ArrayList<VisualItem>());

        aclsListView = (ListView) findViewById(R.id.connectorAppAclsLv);
        aclsListView.setAdapter(adapter);
        aclsListView.setEmptyView(findViewById(R.id.connectorAppAclsLvNoAcls));
        aclsListView.setOnItemClickListener(this);

        retrieveData();
    }

    /**
     * @see android.app.Activity#onStop()
     */
    @Override
    protected void onStop() {
        super.onStop();

        if (asyncTask != null) {
            asyncTask.cancel(true);
        }
    }

    /**
     * @see org.alljoyn.gatewaycontroller.activity.BaseActivity#onSessionJoined()
     */
    @Override
    protected void onSessionJoined() {

        super.onSessionJoined();

        if (invokeOnSessionReady == null) {
            Log.w(TAG, "onSessionJoined is called, but invokeOnSessionReady is undefiend, returning");
            return;
        }

        try {
            invokeOnSessionReady.getMethod().invoke(this, invokeOnSessionReady.getArgs());
        } catch (Exception e) {
            Log.e(TAG, "Failed to invoke the method: '" + invokeOnSessionReady.getMethod().getName() + "'");
            showOkDialog("Error", "Failed to execute an operation", "Ok", null);
        }

        invokeOnSessionReady = null;
    }

    /**
     * @see org.alljoyn.gatewaycontroller.activity.BaseActivity#onSessionJoinFailed()
     */
    @Override
    protected void onSessionJoinFailed() {

        super.onSessionJoinFailed();

        // If the last invoked method was changeAclActiveStatusMethod
        // we need to rollback the Acl activity switch back, because the session
        // establishment has failed
        if (invokeOnSessionReady == null || invokeOnSessionReady.getMethod() != changeAclActiveStatusMethod) {

            return;
        }

        Object[] args                  = invokeOnSessionReady.getArgs();
        final VisualAcl vAcl           = (VisualAcl) args[0];
        final CompoundButton switchAcl = (CompoundButton) args[1];

        vAcl.updateActivityStatus();

        runOnUiThread(new Runnable() {
            @Override
            public void run() {

                switchAcl.setChecked(vAcl.isActive());
            }
        });
    }

    /**
     * @see org.alljoyn.gatewaycontroller.activity.BaseActivity#onGatewayMgmtAnnounced()
     */
    @Override
    protected void onGatewayMgmtAppAnnounced() {

        super.onGatewayMgmtAppAnnounced();

        // Check that my Gateway App wasn't lost because of the GatewayMgmtAppAnnounced
        if (app.getSelectedGatewayApp() == null) {

            return;
        }

        Log.d(TAG, "GatewayListChanged was called, refreshing the activity");
        retrieveData();
    }

    /**
     * @see org.alljoyn.gatewaycontroller.activity.BaseActivity#onSelectedGatewayLost()
     */
    @Override
    protected void onSelectedGatewayLost() {

        super.onSelectedGatewayLost();
        handleLostOfGateway();
    }

    /**
     * Since the application is selected the {@link ConnectorApp} object
     * listens to the incoming status change events
     *
     * @see org.alljoyn.gatewaycontroller.sdk.ConnectorAppStatusSignalHandler#onStatusChanged(java.lang.String,
     *      org.alljoyn.gatewaycontroller.sdk.ConnectorAppStatus)
     */
    @Override
    public void onStatusChanged(String appId, final ConnectorAppStatus status) {

        Log.d(TAG, "Received status changed signal for the app id: '" + appId + "', Status: '" + status + "'");
        if ( !appId.equals(app.getSelectedConnectorApp().getAppId()) ) {

            Log.wtf(TAG, "Weird received status changed for a not selected Connector Application!");
            return;
        }

        runOnUiThread(new Runnable() {
            @Override
            public void run() {

                ConnectorAppsAdapter.updateStatus(connStatus, operStatus, installStatus, status);
            }
        });
    }

    /**
     * @see android.widget.AdapterView.OnItemClickListener#onItemClick(android.widget.AdapterView,
     *      android.view.View, int, long)
     */
    @Override
    public void onItemClick(AdapterView<?> adapter, View clickedView, int position, long rowId) {

        VisualAcl vAcl = (VisualAcl) this.adapter.getItem(position);
        Acl acl        = vAcl.getAcl();
        app.setSelectedAcl(acl);

        Log.d(TAG, "Selected ACL objPath is: '" + acl.getObjectPath() + "'");
        openAcl(AclManagementActivity.ACTIVE_TYPE_ACL_UPDATE);
    }

    /**
     * @see android.app.Activity#onMenuItemSelected(int, android.view.MenuItem)
     */
    @Override
    public boolean onMenuItemSelected(int featureId, MenuItem item) {

        switch (item.getItemId()) {

            case R.id.menuCreateAcl: {

                openAcl(AclManagementActivity.ACTIVE_TYPE_ACL_CREATE);
                return true;
            }
            case R.id.menuConnectorAppRestartBtn: {

                restartApp();
                return true;
            }
            case R.id.menuConnectorAppRefreshBtn: {

                retrieveData();
                return true;
            }
            case R.id.menuShowManifest: {

                showManifest();
                return true;
            }
            default: {

                return super.onMenuItemSelected(featureId, item);
            }
        }
    }

    /**
     * Calls {@link Acl#activate(int)} or
     * {@link Acl#deactivate(int)} in depends on the given
     * isActive flag
     *
     * @param acl
     *            {@link Acl} to activate or deactivate
     * @param aclSwitch
     *            to be affected on the state change
     * @param isActive
     *            The status flag
     */
    public void changeAclActiveStatus(final VisualAcl vAcl, final CompoundButton aclSwitch, final boolean isActive) {

        final Integer sid = getSession();
        if (sid == null) {

            Log.d(TAG, "Can't invoke changeAclActiveStatus, no session with the GW is established, waiting for" +
                           " the onSessionJoined event");

            invokeOnSessionReady = new CallbackMethod(changeAclActiveStatusMethod, new Object[] { vAcl, aclSwitch, isActive });
            return;
        }

        String msg = isActive ? "Activating ACL" : "Deactivating ACL";
        showProgressDialog(msg);

        asyncTask = new AsyncTask<Void, Void, Void>() {

            private String errMsg = null;

            @Override
            protected Void doInBackground(Void... params) {

                try {

                    if (isActive) {
                        vAcl.getAcl().activate(sid);
                    } else {
                        vAcl.getAcl().deactivate(sid);
                    }
                } catch (GatewayControllerException gce) {

                    Log.e(TAG, "Failed to set the ACL state to isActive: '" + isActive + "', objPath: '" +
                                    vAcl.getAcl().getObjectPath() + "'");

                    errMsg = "Failed to change the ACL status";
                }

                return null;
            }

            @Override
            protected void onPostExecute(Void result) {

                hideProgressDialog();

                if (errMsg != null) {
                    showOkDialog("Error", errMsg, "Ok", null);
                }

                vAcl.updateActivityStatus();
                aclSwitch.setChecked(vAcl.isActive());
            }
        };
        asyncTask.execute();

    }// changeAclActiveStatus

    /**
     * Retrieves {@link ConnectorApp} status and its
     * {@link Acl} objects
     */
    private void retrieveData() {

        final Integer sid = getSession();
        if (sid == null) {

            Log.d(TAG, "Can't retrieve Connector application ACLs, no session with the GW is established, waiting for" +
                           " the onSessionJoined event");

            invokeOnSessionReady = new CallbackMethod(retrieveDataMethod, new Object[] {});
            return;
        }

        app.setSelectedAcl(null);
        adapter.clear();
        showProgressDialog("Retrieving ACLs");

        Log.d(TAG, "Retrieving application data");

        asyncTask = new AsyncTask<Void, Void, Void>() {

            private ConnectorAppStatus appStatus;
            private String errMsg;

            @Override
            protected Void doInBackground(Void... params) {

                appStatus = retrieveAppStatusAsyncTask(sid);
                errMsg    = retrieveAclsAsyncTask(sid);
                return null;
            }

            @Override
            protected void onPostExecute(Void result) {

                hideProgressDialog();

                if (errMsg != null) {
                    showOkDialog("Error", errMsg, "Ok", null);
                }

                ConnectorAppsAdapter.updateStatus(connStatus, operStatus, installStatus, appStatus);

                adapter.notifyDataSetChanged();

            }
        };
        asyncTask.execute();
    }// retrieveData

    /**
     * The method is executed on the {@link AsyncTask} thread and retrieves the
     * {@link ConnectorAppStatus} of the selected
     * {@link ConnectorApp}
     *
     * @return {@link ConnectorAppStatus}
     */
    private ConnectorAppStatus retrieveAppStatusAsyncTask(final int sid) {

        try {

            ConnectorApp connApp              = app.getSelectedConnectorApp();
            ConnectorAppStatus status = connApp.retrieveStatus(sid);
            Log.d(TAG, "Retrieved application status for the selectedConnectorApp, objPath: '" +
                        connApp.getObjectPath() + "'" + " status: '" + status + "'");

            return status;
        } catch (GatewayControllerException gce) {
            Log.e(TAG, "Failed to retrieve status of the Selected ConnectorApp., objPath: '" +
                            app.getSelectedConnectorApp().getObjectPath() + "'");

            return null;
        }
    }

    /**
     * The method is executed on the {@link AsyncTask} thread and retrieves the
     * {@link Acl} objects of the selected {@link ConnectorApp}
     * @return Error message if failed
     */
    private String retrieveAclsAsyncTask(int sid) {

        List<Acl> aclList;
        ConnectorApp selConnectorApp = app.getSelectedConnectorApp();

        try {

            aclList = selConnectorApp.retrieveAcls(sid);
        } catch (GatewayControllerException gce) {

            Log.e(TAG, "Failed to retrieve ACL list for the selected ConnectorApp, objPath: '" +
                        selConnectorApp.getObjectPath() + "'", gce);

            return "Failed to retrieve the ACL list.\n Please try later.";
        }

        for (Acl acl : aclList) {

            adapter.add(new VisualAcl(acl));
        }

        return null;
    }

    /**
     * Restart the selected {@link ConnectorApp}
     */
    private void restartApp() {

        final Integer sid = getSession();
        if (sid == null) {

            Log.d(TAG, "Can't restart the Connector Application, no session with the GW is established, waiting for" +
                           " the onSessionJoined event");

            invokeOnSessionReady = new CallbackMethod(restartAppMethod, new Object[] {});
            return;
        }

        showProgressDialog("Restarting application");
        Log.d(TAG, "Restarting application, ConnectorApp: '" + app.getSelectedConnectorApp().getObjectPath() + "'");

        asyncTask = new AsyncTask<Void, Void, Void>() {

            private GatewayControllerException gce;
            private RestartStatus restartStatus;

            @Override
            protected Void doInBackground(Void... params) {

                try {

                    ConnectorApp connApp = app.getSelectedConnectorApp();
                    restartStatus        = connApp.restart(sid);
                    Log.d(TAG, "The app: '" + connApp.getObjectPath() + "' has been restarted, status: '" +
                                    restartStatus + "'");

                } catch (GatewayControllerException gce) {
                    this.gce = gce;
                }
                return null;
            }

            @Override
            protected void onPostExecute(Void result) {

                hideProgressDialog();

                if (gce != null || restartStatus == RestartStatus.GW_RESTART_APP_RC_INVALID) {

                    Log.w(TAG, "Failed to restart the Connector App: '" + app.getSelectedConnectorApp().getObjectPath() +
                               "', restartStatus: '" + restartStatus + "'");

                    showOkDialog("Error", "Failed to restart the application", "Ok", null);
                }
            }
        };
        asyncTask.execute();
    }// retrieveApp

    /**
     * Triggers {@link Intent} to open the {@link AclManagementActivity}
     *
     * @param aclType
     *            {@link AclManagementActivity#ACTIVE_TYPE_ACL_CREATE} or
     *            {@link AclManagementActivity#ACTIVE_TYPE_ACL_UPDATE}
     */
    private void openAcl(int aclType) {

        Intent intent = new Intent(this, AclManagementActivity.class);
        intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP | Intent.FLAG_ACTIVITY_SINGLE_TOP | Intent.FLAG_ACTIVITY_NEW_TASK);
        intent.putExtra(AclManagementActivity.ACTIVE_TYPE_KEY, aclType);
        startActivity(intent);
    }

    /**
     * Show {@link ConnectorApp} manifest
     */
    private void showManifest() {

        Intent intent = new Intent(this, ConnectorAppCapabilitiesActivity.class);
        intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP | Intent.FLAG_ACTIVITY_SINGLE_TOP | Intent.FLAG_ACTIVITY_NEW_TASK);
        startActivity(intent);
    }
}
