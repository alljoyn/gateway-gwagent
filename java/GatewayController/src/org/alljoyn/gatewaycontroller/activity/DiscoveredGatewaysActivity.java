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

import java.math.BigInteger;
import java.security.SecureRandom;
import java.util.ArrayList;
import java.util.List;

import org.alljoyn.bus.alljoyn.DaemonInit;
import org.alljoyn.gatewaycontroller.AuthManager;
import org.alljoyn.gatewaycontroller.R;
import org.alljoyn.gatewaycontroller.adapters.DiscoveredGatewaysAdapter;
import org.alljoyn.gatewaycontroller.adapters.VisualGateway;
import org.alljoyn.gatewaycontroller.adapters.VisualItem;
import org.alljoyn.gatewaycontroller.sdk.GatewayController;
import org.alljoyn.gatewaycontroller.sdk.GatewayControllerException;
import org.alljoyn.gatewaycontroller.sdk.GatewayMgmtApp;

import com.google.gson.Gson;
import com.google.gson.reflect.TypeToken;

import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ListView;

/**
 * Discovered gateways activity
 */
public class DiscoveredGatewaysActivity extends BaseActivity implements OnItemClickListener {
    private static final String TAG = "gwcapp" + DiscoveredGatewaysActivity.class.getSimpleName();

    /**
     * Gateway listview
     */
    private ListView gatewayListView;

    /**
     * Gateways adapter
     */
    private DiscoveredGatewaysAdapter adapter;

    @Override
    protected void passwordRequired() {

    };

    @Override
	protected void setPassCodeFailed(String appId) {
		List<GatewayMgmtApp> gatewayApps = GatewayController.getInstance().getGatewayMgmtApps();
		for (int position = 0; position < gatewayApps.size(); position ++) {
			if (gatewayApps.get(position).getAppId().toString().equals(appId)) {
				((VisualGateway) adapter.getItem(position)).isAuthenticated = false;
				adapter.notifyDataSetChanged();
				break;
			}
		}
	}

    /**
     * @see org.alljoyn.gatewaycontroller.activity.BaseActivity#onCreate(android.os.Bundle)
     */
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.discovered_gateways);

        setTitle(R.string.discovered_gateways_activity);

        // Initialize the AJ daemon
        DaemonInit.PrepareDaemon(this);

        try {
            app.prepareAJ();
        } catch (GatewayControllerException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
    }

    /**
     * @see android.app.Activity#onStart()
     */
    @Override
    protected void onStart() {

        super.onStart();

        gatewayListView = (ListView) findViewById(R.id.gatewaysList);
        List<VisualItem> gateways = new ArrayList<VisualItem>();

        adapter = new DiscoveredGatewaysAdapter(this, R.layout.discovered_gateway_item, gateways);
        gatewayListView.setAdapter(adapter);
        gatewayListView.setEmptyView(findViewById(R.id.discoveredGatewayNoItems));
        gatewayListView.setOnItemClickListener(this);

        retrieveGateways();
    }

    /**
     * @see org.alljoyn.gatewaycontroller.activity.BaseActivity#onGatewayMgmtAnnounced()
     */
    @Override
    protected void onGatewayMgmtAppAnnounced() {
        retrieveGateways();
    }


    /**
     * Retrieve the list of gateways
     */
    private void retrieveGateways() {

        SharedPreferences sharedPreferences = getSharedPreferences(AuthManager.SHARED_PREFERENCES_PASSCODES_NAME, MODE_PRIVATE);

        List<GatewayMgmtApp> gatewayApps = GatewayController.getInstance().getGatewayMgmtApps();


        adapter.clear();
        for (GatewayMgmtApp gw : gatewayApps) {

            //check if passcode is stored. If it is not stored, then set it.
            // I commented out the if statement, because it did not include behavior for when
            // passcode is changed from the configfile.
            
            //if (sharedPreferences.getString(gw.getAppId().toString(), "").equals("")) {
                app.setGatewayPasscode((new BigInteger(160, new SecureRandom())).toString(32), gw.getBusName(), gw.getAppId().toString());
            //}
           
            adapter.add(new VisualGateway(gw));
        }

        Log.d(TAG, "Found gateways: '" + gatewayApps + "'");
        adapter.notifyDataSetChanged();
    }

    /**
     * @see android.widget.AdapterView.OnItemClickListener#onItemClick(android.widget.AdapterView,
     *      android.view.View, int, long)
     */
    @Override
    public void onItemClick(AdapterView<?> adapter, View clickedView, int position, long rowId) {

        VisualGateway vg = (VisualGateway) this.adapter.getItem(position);

        app.setSelectedGatewayApp(vg.getGateway());

        // If we have an old session, we need to close it when selecting a new
        // gateway
        app.leaveSession();

        Intent intent = new Intent(this, ConnectorAppsActivity.class);
        intent.setFlags(
                Intent.FLAG_ACTIVITY_CLEAR_TOP | Intent.FLAG_ACTIVITY_SINGLE_TOP | Intent.FLAG_ACTIVITY_NEW_TASK);
        startActivity(intent);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        app.cleanUpAJ();
    }

}
