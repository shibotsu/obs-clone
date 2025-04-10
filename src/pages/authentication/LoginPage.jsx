import { useState, useRef, useEffect } from "react";
import { useNavigate } from "react-router-dom";
import { TextField, Stack, PrimaryButton } from "@fluentui/react";
import { Text, MessageBar, Spinner } from "@fluentui/react-components";
import "./Auth.css";
import { useAuth } from "../../context/AuthContext";
import { Link } from "react-router-dom";
import authUseStyles from "./AuthUseStyles";

const LoginPage = () => {
  const classes = authUseStyles();

  const { login } = useAuth(); // will be used after implementing the backend
  const [password, setPassword] = useState("");
  const [usernameOrEmail, setUsernameOrEmail] = useState("");
  const [error, setError] = useState("");
  const [isLoading, setIsLoading] = useState(false);

  const userRef = useRef();

  let navigate = useNavigate();

  const handleSubmit = async (e) => {
    e.preventDefault();

    // create request payload
    const requestData = {
      login: usernameOrEmail,
      password: password,
    };

    try {
      setIsLoading(true);
      const response = await fetch("http://127.0.0.1:8000/api/login", {
        method: "POST",
        headers: {
          "Content-Type": "application/json",
        },
        body: JSON.stringify(requestData),
      });

      if (!response.ok) {
        const errorData = await response.json();
        setError(errorData.message || "Login failed");
        return;
      }

      // parsing the JSON response
      const data = await response.json();
      {
        console.log(JSON.stringify(data));
      }
      const { token, user } = data;

      login(token, user);
      console.log(JSON.stringify(user));
      navigate("/");
    } catch (error) {
      setError("Login failed: " + error);
    } finally {
      setIsLoading(false);
    }
  };

  useEffect(() => {
    if (userRef.current) {
      userRef.current.focus();
    }
  }, []);

  return (
    <div>
      <form onSubmit={handleSubmit} className={classes.loginContainer}>
        <Stack tokens={{ childrenGap: 10 }} className={classes.stack}>
          <TextField
            componentRef={userRef}
            label="Username or Email"
            value={usernameOrEmail}
            onChange={(e, newValue) => setUsernameOrEmail(newValue || "")}
            required
            className={classes.textfield}
            styles={{
              fieldGroup: { height: "40px !important" },
            }}
          />
          <TextField
            label="Password"
            type="password"
            value={password}
            onChange={(e, newValue) => setPassword(newValue || "")}
            required
            lassName={classes.textfield}
            styles={{
              fieldGroup: { height: "40px !important" },
            }}
          />
          {isLoading ? (
            <Spinner />
          ) : (
            <PrimaryButton
              text="Login"
              type="submit"
              className={classes.button}
            />
          )}
          {error && <MessageBar intent="error">{error}</MessageBar>}
          <div className={classes.alreadyRegistered}>
            <Text>Not signed up?</Text>
            <Link className={classes.link} to={"/register"}>
              Register
            </Link>
          </div>
        </Stack>
      </form>
    </div>
  );
};

export default LoginPage;
